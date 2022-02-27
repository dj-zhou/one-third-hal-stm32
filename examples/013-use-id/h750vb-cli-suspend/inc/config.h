#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

// ============================================================================
// one-third-core includes
// ----------------------------------------------------------------------------
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_1K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_USART1_PA9PA10
#define _CLI_OUT_MESSAGE "h750vb"
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOC
#define _LED_HEARTBEAT_PIN 13
#define _LED_HEARTBEAT_TASK_MS 10
#define _LED_LOW_DRIVE
#include "led-status.h"

// ----------------------------------------------------------------------------
#define _USE_ID
#include "id.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
