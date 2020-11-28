#ifndef __GENERAL_UTILS_H
#define __GENERAL_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"  // must be on the top

#include "config-gpio.h"
#include "config-spi.h"
#include "config-timer.h"
#include "config-usart.h"

#if defined(STM32F030x8)
#include "stm32f0xx.h"
#elif defined(STM32F103xB) || defined(STM32F107xC)
#include "stm32f1xx.h"
#elif defined(STM32F407xx) || defined(STM32F427xx)
#include "stm32f4xx.h"
#elif defined(STM32F767xx)
#include "stm32f7xx.h"
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// clang-format off
// 0x1234 --> 0x3412
#if !defined(_SWAP_16)
    #define _SWAP_16(x) (x = (uint16_t)((x >> 8) | (x << 8)))
#endif

// 0x 12345678 --> 0x78563412
#if !defined(_SWAP_32)
    #define _SWAP_32(x)                                                     \
        (x = ((x >> 24) | ((x >> 8) & 0x0000ff00) | ((x << 8) & 0x00ff0000) \
            | (x << 24)))
#endif

#if !defined(_CHECK_BIT)
    #define _CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#endif

#if !defined(_SET_BIT)
    #define _SET_BIT(var, pos) ((var) |= (1 << (pos)))
#endif

#if !defined(_RESET_BIT)
    #define _RESET_BIT(var, pos) ((var) &= (~(1 << (pos))))
#endif

#if !defined(_SIZE_OF_ARRAY)
    #define _SIZE_OF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))
#endif

#if !defined(offsetof)
#define offsetof(TYPE, MEMBER) ((size_t) & (( TYPE* )0)->MEMBER)
#endif

#if !defined(container_of)
#define container_of(ptr, type, member)                      \
    ({                                                       \
        const typeof((( type* )0)->member)* __mptr = (ptr);  \
        ( type* )(( char* )__mptr - offsetof(type, member)); \
    })
#endif
// clang-format on

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
    FREERTOS_NOSTART = (( uint8_t )1),
    FREERTOS_STARTED = (( uint8_t )2),
} RtosState_e;

typedef struct {
    void (*setState)(RtosState_e state);
    RtosState_e (*getState)(void);
} Rtos;
#endif  // RTOS_IS_USED

typedef struct {
    HAL_StatusTypeDef (*initClock)(uint16_t, uint16_t, uint16_t);
    void (*initNvic)(uint8_t group);
} System;

typedef struct {
    void (*enableGpio)(GPIO_TypeDef* GPIOx);
    void (*enableIic)(I2C_TypeDef* I2Cx);
    void (*enableSpi)(SPI_TypeDef* SPIx);
    void (*enableTimer)(TIM_TypeDef* TIMx);
    void (*enableUart)(USART_TypeDef* USARTx);
} Clock;

typedef struct {
    void (*mode)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t mode);
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    void (*alter)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint8_t alt);
#endif
    void (*pull)(GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p);
    void (*set)(GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v);
    void (*toggle)(GPIO_TypeDef* GPIOx, uint8_t pin_n);
} Pin;

// ============================================================================
// clang-format off
typedef struct {
    System system ;
    Clock  clock  ;
    Pin    pin    ;
#if defined(RTOS_IS_USED)
    Rtos rtos;
#endif
} UtilsApi_t;
extern UtilsApi_t utils;
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __GENERAL_UTILS_H
