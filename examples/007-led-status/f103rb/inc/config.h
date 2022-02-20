#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

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
// #define _CONSOLE_USE_USART3_PC10PC11
// #define _CONSOLE_USE_USART1_PB6PB7
// #define _CONSOLE_USE_UART5_PC12PD2
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOA
#define _LED_HEARTBEAT_PIN 5
#define _LED_HEARTBEAT_TASK_MS 5
#include "led-status.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
