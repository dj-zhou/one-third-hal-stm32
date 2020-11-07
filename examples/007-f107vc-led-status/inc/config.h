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
// #define _CONSOLE_USE_UART1_PA9PA10
#define _CONSOLE_USE_UART1_PB6PB7
// #define _CONSOLE_USE_UART2_PA2PA3
// #define _CONSOLE_USE_UART2_PD5PD6
// #define _CONSOLE_USE_UART3_PB10PB11
// #define _CONSOLE_USE_UART3_PC10PC11
// #define _CONSOLE_USE_UART3_PD8PD9
// #define _CONSOLE_USE_UART4_PC10PC11
// #define _CONSOLE_USE_UART5_PC12PD2
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOC
#define _LED_HEARTBEAT_PIN 6
#define _LED_HEARTBEAT_BLINK 5
#include "led-status.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
