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
#define _STIME_200_TICK
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
#define _USE_IIC1_PB8PB9
#include "iic.h"
#define _EEPROM_USE_AT24C08
#define _AT24C08_ON_IIC1
#include "iic-eeprom.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
