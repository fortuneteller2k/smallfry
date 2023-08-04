#pragma once

#include <FreeRTOS.h>
#include <croutine.h>
#include <task.h>

#include <cstdio>
#include <utility>

#include "pico/platform.h"

extern "C" {

#define coroutine_delay_ms(handle, ms) crDELAY(handle, ms);
#define task_delay_ms(ms) vTaskDelay(pdMS_TO_TICKS(ms))

void vApplicationIdleHook() {
  vCoRoutineSchedule();
  asm volatile("wfi");
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) {
  (void)pxTask;

  printf("%s encountered stack overflow!\n", pcTaskName);

  taskDISABLE_INTERRUPTS();
  asm volatile("bkpt #0");

  for (;;) tight_loop_contents();
}

void vApplicationMallocFailedHook() {
  taskDISABLE_INTERRUPTS();
  asm volatile("bkpt #0");

  for (;;) tight_loop_contents();
}
}