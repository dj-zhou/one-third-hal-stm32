#pragma once

#include "config.h"
#include <stdbool.h>

// uint16_t g_config_uart_used;

// clang-format off

// commonly existed ones -------------
#define USART1_EXISTS
#define USART2_EXISTS

// special ones ---------------
#if defined(STM32F103xB)
    #define USART3_EXISTS
#endif

#if defined(STM32F107xC) || defined(STM32F303xE)
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
#endif

#if defined(STM32F407xx)
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
#endif

#if defined(STM32F410Rx)
    #define USART6_EXISTS
#endif

#if defined(STM32F446xx)
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
#endif

#if defined(STM32F427xx) || defined(STM32F746xx) || defined(STM32F767xx)
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
    #define UART7_EXISTS
    #define UART8_EXISTS
#endif

#if defined(STM32H750xx)
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
    #define UART7_EXISTS
    #define UART8_EXISTS
#endif

#if defined(USART1_EXISTS) || defined(USART2_EXISTS) || defined(USART3_EXISTS) \
 || defined(UART4_EXISTS)  || defined(UART5_EXISTS)  || defined(USART6_EXISTS) \
 || defined(UART7_EXISTS)  || defined(UART8_EXISTS)
    #define UART_EXISTS
#endif
// clang-format on

typedef struct {
    void (*set)(USART_TypeDef* USARTx, bool value);
    bool (*check)(USART_TypeDef* USARTx);
} ConfigUarts_t;

extern ConfigUarts_t config_uarts;
