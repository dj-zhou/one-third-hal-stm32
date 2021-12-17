#ifndef __CONFIG_H
#define __CONFIG_H

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
#define _STIME_1K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
// #define _CONSOLE_USE_USART1_PA9PA10
#define _CONSOLE_USE_USART2_PA2PA3
// #define _CONSOLE_USE_USART3_PC10PC11
// #define _CONSOLE_USE_USART1_PB6PB7
// #define _CONSOLE_USE_UART5_PC12PD2
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOE
#define _LED_HEARTBEAT_PIN 11
#define _LED_HEARTBEAT_TASK_MS 5
#include "led-status.h"

// ----------------------------------------------------------------------------
#define _USE_CAN1_PD1PD0
#define _CAN1_IRQ_SHOW_UNKNOWN_MSG
#define _USE_CAN2_PB13PB12
#include "can.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
