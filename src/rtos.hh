#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <cstdio>
#include <utility>
#include "pico/platform.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  (void)xTask;

  printf("%s encountered stack overflow!\n", pcTaskName);
  for (;;) tight_loop_contents();
  return std::unreachable();
}