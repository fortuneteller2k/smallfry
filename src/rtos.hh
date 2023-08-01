#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <cstdio>
#include <utility>
#include "pico/platform.h"

extern "C" {

void vApplicationIdleHook(void) {
  asm volatile("wfi"); // wait for interrupt
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName) {
  (void)pxTask;

  printf("%s encountered stack overflow!\n", pcTaskName);

  taskDISABLE_INTERRUPTS();
  asm volatile("bkpt #0");
  
  for (;;) tight_loop_contents();
  return std::unreachable();
}

void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  asm volatile("bkpt #0");
  
  for (;;) tight_loop_contents();
  return std::unreachable();
}

}