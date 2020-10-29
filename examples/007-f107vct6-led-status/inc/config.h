#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

// ============================================================================
// ----------------------------------------------------------------------------
// one-third-core includes
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_4K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_UART5_PC12PD2
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOD
#define _LED_HEARTBEAT_PIN 3
#define _LED_HEARTBEAT_BLINK 5
#include "led-status.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
