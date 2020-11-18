#ifndef __CONFIG_USART_H
#define __CONFIG_USART_H

#include "config.h"

uint16_t g_config_usart_used;

// clang-format off

// commonly existed ones -------------
#define USART1_EXISTS
#define USART2_EXISTS

// special ones ---------------
#if defined( STM32F103xB )
    #define USART3_EXISTS
#endif

#if defined( STM32F107xC ) || defined( STM32F303xE )
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
#endif

#if defined( STM32F407xx )
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
#endif

#if defined( STM32F427xx ) || defined ( STM32F746xx ) || defined ( STM32F767xx )
    #define USART3_EXISTS
    #define UART4_EXISTS
    #define UART5_EXISTS
    #define USART6_EXISTS
    #define UART7_EXISTS
    #define UART8_EXISTS
#endif

// clang-format on

#endif  // __CONFIG_USART_H
