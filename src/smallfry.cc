#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "boards/adafruit_feather_rp2040.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/regs/intctrl.h"
#include "hardware/vreg.h"
#include "mfrc522.hh"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/platform.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "protothreads.h"
#include "ws2812.pio.h"
}

#include "rgb8.hh"
#include "mfrc522.hh"

#define OVERCLOCK_273_MHZ true
#define UNDERCLOCK_18_MHZ true

spin_lock_t *lock_delta, *lock_error;

void pwm_isr_on_wrap() {
  pwm_clear_irq(pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN));

  static uint16_t x = 0;
  float n = 500;
  float mu = 0.5;
  float sigma = 0.15;

  uint16_t duty = (uint16_t)UINT16_MAX * (0.5 * expf(-(powf(((x / n) - mu) / sigma, 2.0))));
  x = x >= n ? 0 : ++x;

  pwm_set_gpio_level(PICO_DEFAULT_LED_PIN, duty);
}

static PT_THREAD(pt_pwm(struct pt *pt)) {
  PT_BEGIN(pt);

  gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);

  uint8_t pwm_slice = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

  pwm_clear_irq(pwm_slice);
  pwm_set_irq_enabled(pwm_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_isr_on_wrap);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  pwm_config config = pwm_get_default_config();

  pwm_config_set_clkdiv(&config, 10.0);
  pwm_init(pwm_slice, &config, true);

  for (;;)
    tight_loop_contents();

  PT_END(pt);
}

static PT_THREAD(pt_onboard_ws2812(struct pt *pt)) {
  PT_BEGIN(pt);

  PIO pio = pio0;
  uint32_t state_machine = 0;
  uint32_t offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, state_machine, offset, PICO_DEFAULT_WS2812_PIN, 800000, true);

  for (uint8_t i = 0; true; i == UINT8_MAX ? i = 0 : i++) {
    uint32_t color = RGB8::rgb8_as_u32(RGB8().wheel(i).brightness(30));
    pio_sm_put_blocking(pio, state_machine, color << 8);
    sleep_ms(10);
  }

  PT_END(pt);
}

static PT_THREAD(pt_mfrc522_test(struct pt *pt)) {
  PT_BEGIN(pt);

  MFRC522 device = MFRC522(
    PICO_DEFAULT_SPI_SCK_PIN,
    PICO_DEFAULT_SPI_TX_PIN,
    PICO_DEFAULT_SPI_RX_PIN,
    25,
    6,
    spi0
  );

  device.init();
  bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI))
  bi_decl(bi_1pin_with_name(25, "SPI chip select"));

  sleep_ms(10000);
  
  if (device.self_test())
    printf("self-test passed!\n");
  else
    printf("self-test failed!\n");

  PT_END(pt);
}

void core1_entry() {
  pt_add_thread(pt_mfrc522_test);
  pt_add_thread(pt_pwm);
  pt_schedule_start;
}

int main(void) {
#if OVERCLOCK_273_MHZ
  vreg_set_voltage(VREG_VOLTAGE_1_30);
  set_sys_clock_khz(273000, true);
#elif UNDERCLOCK_18_MHZ
  vreg_set_voltage(VREG_VOLTAGE_0_90);
  set_sys_clock_khz(18000, true);
#endif

  stdio_init_all();
  setup_default_uart();

  PT_LOCK_INIT(lock_error, 31, UNLOCKED);
  PT_LOCK_INIT(lock_delta, 30, UNLOCKED);

  multicore_reset_core1();
  multicore_launch_core1(&core1_entry);

  pt_add_thread(pt_onboard_ws2812);
  pt_schedule_start;
}