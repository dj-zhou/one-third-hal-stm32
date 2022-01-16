#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// ============================================================================
// one-third-core includes
// ----------------------------------------------------------------------------
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_4K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_USART2_PA2PA3
#define _CLI_OUT_MESSAGE "comm"
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOE
#define _LED_HEARTBEAT_PIN 11
#include "led-status.h"

#define _USE_USART1_PA9PA10
#include "uart/uart.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
