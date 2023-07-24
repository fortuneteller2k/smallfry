#include "boards/adafruit_feather_rp2040.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/regs/intctrl.h"
#include "hardware/vreg.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "pico/platform.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "protothreads.h"
#include "ws2812.pio.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OVERCLOCK_273_MHZ true
#define UNDERCLOCK_18_MHZ true

spin_lock_t *lock_delta, *lock_error;

typedef struct RGB8 {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB8;

RGB8 wheel(uint8_t pos) {
  RGB8 color;

  pos = UINT8_MAX - pos;

  if (pos < 85) {
    color.r = UINT8_MAX - pos * 3;
    color.g = 0;
    color.b = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    color.r = 0;
    color.g = pos * 3;
    color.b = UINT8_MAX - pos * 3;
  } else {
    pos -= 170;
    color.r = pos * 3;
    color.g = UINT8_MAX - pos * 3;
    color.b = 0;
  }

  return color;
}

RGB8 brightness(RGB8 color, uint8_t brightness) {
  color.r = (uint8_t)((uint16_t)color.r * ((uint16_t)brightness + 1) /
                      (UINT8_MAX + 1));
  color.g = (uint8_t)((uint16_t)color.g * ((uint16_t)brightness + 1) /
                      (UINT8_MAX + 1));
  color.b = (uint8_t)((uint16_t)color.b * ((uint16_t)brightness + 1) /
                      (UINT8_MAX + 1));

  return color;
}

static inline uint32_t rgb_as_u32(RGB8 color) {
  return ((uint32_t)(color.r) << 8) | ((uint32_t)(color.g) << 16) |
         (uint32_t)(color.b);
}

void pwm_isr_on_wrap() {
  pwm_clear_irq(pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN));

  static uint16_t x = 0;
  float n = 500;
  float mu = 0.5;
  float sigma = 0.15;

  uint16_t duty =
      (uint16_t)UINT16_MAX * (0.5 * expf(-(powf(((x / n) - mu) / sigma, 2.0))));
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

  ws2812_program_init(pio, state_machine, offset, PICO_DEFAULT_WS2812_PIN,
                      800000, true);

  for (uint8_t i = 0; true; i == UINT8_MAX ? i = 0 : i++) {
    uint32_t color = rgb_as_u32(brightness(wheel(i), 30));
    pio_sm_put_blocking(pio, state_machine, color << 8);
    sleep_ms(10);
  }

  PT_END(pt);
}

void core1_entry() {
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