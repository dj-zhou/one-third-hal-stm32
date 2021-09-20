#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// ============================================================================
// one-third-core includes
// ----------------------------------------------------------------------------
// #include F427XX_HCLK_176M 176
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_200_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
// #define _CONSOLE_USE_USART1_PA9PA10
// #define _CONSOLE_USE_USART2_PA2PA3
// #define _CONSOLE_USE_USART3_PC10PC11
// #define _CONSOLE_USE_USART1_PB6PB7
// #define _CONSOLE_USE_UART5_PC12PD2
#define _CONSOLE_USE_UART7_PE8PE7
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOB
#define _LED_HEARTBEAT_PIN 3  // 1, 3, 11
#define _LED_HEARTBEAT_TASK_MS 5
#define _LED_LOW_DRIVE
#include "led-status.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
