#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

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
#define _CONSOLE_USE_USART3_PD8PD9
#define _CLI_OUT_MESSAGE "ringbuffer-f767zi"
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOB
#define _LED_HEARTBEAT_PIN 0
#define _LED_HEARTBEAT_TASK_MS 5
#include "led-status.h"

// ----------------------------------------------------------------------------
#include "operation.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
