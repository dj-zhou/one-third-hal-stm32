#pragma once

#ifdef __cplusplus
extern "C" {
#endif
// ============================================================================
// must be on the top
#include "config.h"

#include "config-can.h"
#include "config-gpio.h"
#include "config-iic.h"
#include "config-spi.h"
#include "config-timer.h"
#include "config-uart.h"
#include "general.h"

#if defined(STM32F030x8)
#include "../CMSIS/Device/ST/STM32F0xx/Include/stm32f0xx.h"
#elif defined(STM32F103xB) || defined(STM32F107xC)
#include "../CMSIS/Device/ST/STM32F1xx/Include/stm32f1xx.h"
#elif defined(STM32F303xE)
#include "../CMSIS/Device/ST/STM32F3xx/Include/stm32f3xx.h"
#elif defined(STM32F407xx) || defined(STM32F427xx)
#include "../CMSIS/Device/ST/STM32F4xx/Include/stm32f4xx.h"
#elif defined(STM32F746xx) || defined(STM32F767xx)
#include "../CMSIS/Device/ST/STM32F7xx/Include/stm32f7xx.h"
#elif defined(STM32H750xx)
#include "../CMSIS/Device/ST/STM32H7xx/Include/stm32h7xx.h"
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// RTOS related macros

// clang-format off
// FreeRTOS must use any of these two macros --------------------
// configUSE_TIME_SLICING
// configUSE_PREEMPTION
#if defined(configUSE_TIME_SLICING) || defined(configUSE_PREEMPTION)
    #if !defined(RTOS_USE_FREERTOS)
        #define RTOS_USE_FREERTOS
    #endif
#endif
// some other RTOS related macros list here

// other RTOS can be added behind
#if defined(RTOS_USE_FREERTOS)
    #define RTOS_IS_USED
#endif
// clang-format on

#if defined(RTOS_IS_USED)
// this allows for adding other RTOS
typedef enum {
    FREERTOS_NOSTART = ((uint8_t)1),
    FREERTOS_STARTED = ((uint8_t)2),
} RtosState_e;

typedef struct {
    void (*setState)(RtosState_e state);
    RtosState_e (*getState)(void);
} UtilsRtos;
#endif  // RTOS_IS_USED

#if defined(STM32F030x8)
#define SYSTEM_CLOCK_HAS_APB1
#endif
#if defined(STM32F103xB) || defined(STM32F107xC) || defined(STM32F303xE)    \
    || defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
#define SYSTEM_CLOCK_HAS_APB12
#endif
#if defined(STM32H750xx)
#define SYSTEM_CLOCK_HAS_APB1234
#endif

typedef struct {
#if defined(SYSTEM_CLOCK_HAS_APB1)
    HAL_StatusTypeDef (*initClock)(uint16_t, uint16_t);
#elif defined(SYSTEM_CLOCK_HAS_APB12)
    HAL_StatusTypeDef (*initClock)(uint16_t, uint16_t, uint16_t);
#elif defined(SYSTEM_CLOCK_HAS_APB1234)
    HAL_StatusTypeDef (*initClock)(uint16_t, uint16_t, uint16_t);
#endif
    void (*initNvic)(uint8_t group);
} UtilsSystem;

typedef struct {
#if defined(CAN_EXISTS)
    void (*enableCan)(CAN_TypeDef* CANx);
#endif
    void (*enableGpio)(GPIO_TypeDef* GPIOx);
#if defined(IIC_EXISTS)
    void (*enableIic)(I2C_TypeDef* I2Cx);
#endif
#if defined(SPI_EXISTS)
    void (*enableSpi)(SPI_TypeDef* SPIx);
#endif
    void (*enableTimer)(TIM_TypeDef* TIMx);
#if defined(UART_EXISTS)
    void (*enableUart)(USART_TypeDef* USARTx);
#endif
} UtilsClock;

typedef struct {
    void (*mode)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t mode);
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    void (*alter)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint8_t alt);
#endif
    void (*pull)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p);
    void (*set)(GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v);
    void (*toggle)(GPIO_TypeDef* GPIOx, uint8_t pin_n);
    bool (*read)(GPIO_TypeDef* GPIOx, uint8_t pin_n);
} UtilsPin;

// ============================================================================
// clang-format off
typedef struct {
    UtilsSystem system ;
    UtilsClock  clock  ;
    UtilsPin    pin    ;
#if defined(RTOS_IS_USED)
    UtilsRtos rtos;
#endif
} UtilsApi_t;
extern UtilsApi_t utils;
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif
