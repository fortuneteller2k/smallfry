#include <FreeRTOS.h>
#include <task.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

#include "boards/adafruit_feather_rp2040.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/regs/intctrl.h"
#include "hardware/vreg.h"
#include "mfrc522.hh"
#include "pico/binary_info.h"
#include "pico/platform.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "rgb8.hh"
#include "rtos.hh"
#include "ws2812.pio.h"

#define MFRC522_ENABLE false

TaskHandle_t ws2812_handle, pwm_handle, button_handle;
uint32_t ws2812_enable = true;

void onboard_button_task(void*) {
  gpio_init(4);
  gpio_set_dir(4, GPIO_IN);
  gpio_pull_up(4);

  bool btn_last = gpio_get(4);

  for (;;) {
    bool btn_current = gpio_get(4);

    if (!btn_current && btn_last) ws2812_enable = !ws2812_enable;

    btn_last = btn_current;
    xTaskNotify(ws2812_handle, ws2812_enable, eSetValueWithOverwrite);
    task_delay_ms(5);
  }

  return vTaskDelete(nullptr);
}

void onboard_ws2812_task(void*) {
  PIO pio = pio0;
  uint32_t state_machine = 0;
  uint32_t offset = pio_add_program(pio, &ws2812_program);

  gpio_set_drive_strength(PICO_DEFAULT_WS2812_PIN, GPIO_DRIVE_STRENGTH_12MA);
  ws2812_program_init(pio, state_machine, offset, PICO_DEFAULT_WS2812_PIN, 800000, true);

  for (uint8_t i = 0; true; i == UINT8_MAX ? i = 0 : i++) {
    if (xTaskNotifyWait(0, 0, &ws2812_enable, portMAX_DELAY)) {
      uint32_t color = RGB8::rgb8_as_u32(RGB8::wheel(i).brightness(UINT8_MAX));

      if (ws2812_enable)
        pio_sm_put_blocking(pio, state_machine, color << 8);
      else
        pio_sm_put_blocking(pio, state_machine, 0);
    }

    task_delay_ms(10);
  }

  return vTaskDelete(nullptr);
}

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

void onboard_led_pwm_task(void*) {
  gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
  gpio_set_drive_strength(PICO_DEFAULT_LED_PIN, GPIO_DRIVE_STRENGTH_12MA);
  gpio_set_slew_rate(PICO_DEFAULT_LED_PIN, GPIO_SLEW_RATE_FAST);

  uint8_t pwm_slice = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

  pwm_clear_irq(pwm_slice);
  pwm_set_irq_enabled(pwm_slice, true);
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_isr_on_wrap);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  pwm_config config = pwm_get_default_config();

  pwm_config_set_clkdiv(&config, 5.0);
  pwm_init(pwm_slice, &config, true);

  for (;;) tight_loop_contents();

  return vTaskDelete(nullptr);
}

void onboard_temp_coroutine(CoRoutineHandle_t handle, UBaseType_t) {
  crSTART(handle);

  static double voltage, temp;

  adc_init();
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);

  for (;;) {
    voltage = adc_read() * (3.3 / (1 << 12));
    temp = 27 - (voltage - 0.706) / 0.001721;

    printf("%.2f °C\n", temp);
    coroutine_delay_ms(handle, 1000);
  }

  crEND();
}

#if MFRC522_ENABLE
void mfrc522_task(void*) {
  MFRC522 device = MFRC522(PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, 25, 6, spi0);

  task_delay_ms(5000);

  if (device.self_test())
    printf("self test passed!\n");
  else
    printf("self test failed!\n");

  return vTaskDelete(nullptr);
}
#endif

int main() {
  vreg_set_voltage(VREG_VOLTAGE_1_30);
  set_sys_clock_khz(configCPU_CLOCK_HZ / 1000, true);

  stdio_init_all();

  TaskHandle_t core0, core1, all_cores;

  xTaskCreate(onboard_button_task, "onboard_button_task", configMINIMAL_STACK_SIZE, nullptr, 1, &button_handle);
  xTaskCreate(onboard_ws2812_task, "onboard_ws2812_task", configMINIMAL_STACK_SIZE, nullptr, 1, &ws2812_handle);
  xTaskCreate(onboard_led_pwm_task, "onboard_led_pwm_task", configMINIMAL_STACK_SIZE, nullptr, 1, &pwm_handle);
  xCoRoutineCreate(onboard_temp_coroutine, 0, 0);

#if MFRC522_ENABLE
  xTaskCreate(mfrc522_task, "mfrc522_task", 4096, nullptr, 5, &core0);
#endif

  vTaskCoreAffinitySet(pwm_handle, (1 << 0));
  vTaskCoreAffinitySet(button_handle, (1 << 1));
  vTaskCoreAffinitySet(ws2812_handle, (1 << 0) | (1 << 1));

  vTaskStartScheduler();

  std::unreachable();
}
