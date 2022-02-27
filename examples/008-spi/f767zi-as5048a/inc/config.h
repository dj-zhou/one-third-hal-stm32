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
#define _STIME_2K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
// #define _CONSOLE_USE_USART1_PA9PA10
// #define _CONSOLE_USE_USART2_PA2PA3
// #define _CONSOLE_USE_USART3_PC10PC11
#define _CONSOLE_USE_USART3_PD8PD9
// #define _CONSOLE_USE_USART1_PB6PB7
// #define _CONSOLE_USE_UART5_PC12PD2
// #define _CONSOLE_USE_UART7_PE8PE7
#include "uart-console.h"

// ----------------------------------------------------------------------------
#define _LED_HEARTBEAT_PORT GPIOB
#define _LED_HEARTBEAT_PIN 7
#define _LED_HEARTBEAT_TASK_MS 10
#include "led-status.h"

// ----------------------------------------------------------------------------
#define _USE_SPI1_PA7PA6
#include "spi.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
