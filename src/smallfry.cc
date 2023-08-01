#include <FreeRTOS.h>
#include <task.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "rtos.hh"
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
#include "rgb8.hh"
#include "ws2812.pio.h"

#define OVERCLOCK_273_MHZ true
#define UNDERCLOCK_18_MHZ false

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

void onboard_led_pwm_task(void* p) {
  gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
  gpio_set_drive_strength(PICO_DEFAULT_LED_PIN, GPIO_DRIVE_STRENGTH_12MA);
  gpio_set_slew_rate(PICO_DEFAULT_LED_PIN, GPIO_SLEW_RATE_FAST);

  uint8_t pwm_slice = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

  pwm_clear_irq(pwm_slice);
  pwm_set_irq_enabled(pwm_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_isr_on_wrap);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  pwm_config config = pwm_get_default_config();

  pwm_config_set_clkdiv(&config, 10.0);
  pwm_init(pwm_slice, &config, true);

  for (;;) tight_loop_contents();

  return std::unreachable();
}

void onboard_ws2812_task(void* p) {
  PIO pio = pio0;
  uint32_t state_machine = 0;
  uint32_t offset = pio_add_program(pio, &ws2812_program);

  gpio_set_drive_strength(PICO_DEFAULT_WS2812_PIN, GPIO_DRIVE_STRENGTH_12MA);
  ws2812_program_init(pio, state_machine, offset, PICO_DEFAULT_WS2812_PIN, 800000, true);

  for (uint8_t i = 0; true; i == UINT8_MAX ? i = 0 : i++) {
    uint32_t color = RGB8::rgb8_as_u32(RGB8::wheel(i).brightness(10));
    pio_sm_put_blocking(pio, state_machine, color << 8);
    vTaskDelay(5);
  }

  return std::unreachable();
}

void mfrc522_task(void* p) {
  MFRC522 device = MFRC522(PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, 25, 6, spi0);

  vTaskDelay(5000);

  if (device.self_test())
    printf("self test passed!\n");
  else
    printf("self test failed!\n");
}

int main() {
#if OVERCLOCK_273_MHZ
  vreg_set_voltage(VREG_VOLTAGE_1_30);
  set_sys_clock_khz(273000, true);
#elif UNDERCLOCK_18_MHZ
  vreg_set_voltage(VREG_VOLTAGE_0_90);
  set_sys_clock_khz(18000, true);
#endif

  stdio_init_all();
  setup_default_uart();

  TaskHandle_t core0_handle, core1_handle;

  xTaskCreate(onboard_ws2812_task, "onboard_ws2812_task", 1024, NULL, 1, &core0_handle);
  xTaskCreate(mfrc522_task, "mfrc522_task", 4096, NULL, 5, &core1_handle);
  xTaskCreate(onboard_led_pwm_task, "onboard_led_pwm_task", 1024, NULL, 1, &core1_handle);

  vTaskCoreAffinitySet(core0_handle, (1 << 0));
  vTaskCoreAffinitySet(core1_handle, (1 << 1));

  vTaskStartScheduler();

  std::unreachable();
  return EXIT_FAILURE;
}