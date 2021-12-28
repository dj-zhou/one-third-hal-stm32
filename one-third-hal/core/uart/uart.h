#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif
// ============================================================================
#include "config.h"

#include "config-uart.h"
#include "general.h"
#include "ring-buffer.h"

// ============================================================================
// clang-format off
// ----------------------------------------------------------------------------
#if defined(USART1_EXISTS)
    #if defined(_USE_USART1_PA9PA10) || defined(_USE_USART1_PB6PB7)
        #define USART1_IS_USED
    #endif
#endif

#if defined(USART2_EXISTS)
    #if defined(_USE_USART2_PA2PA3) || defined(_USE_USART2_PD5PD6)
        #define USART2_IS_USED
    #endif
#endif

#if defined(USART3_EXISTS)
    #if defined(_USE_USART3_PB10PB11) || defined(_USE_USART3_PC10PC11) \
     || defined(_USE_USART3_PD8PD9)
        #define USART3_IS_USED
    #endif
#endif

#if defined(UART4_EXISTS)
    #if defined(_USE_UART4_PC10PC11)
        #define UART4_IS_USED
    #endif
#endif

#if defined(UART5_EXISTS)
    #if defined(_USE_UART5_PC12PD2)
        #define UART5_IS_USED
    #endif
#endif

#if defined(USART6_EXISTS)
    // todo
#endif


#if defined(UART7_EXISTS)
    // todo
#endif

#if defined(UART8_EXISTS)
    // todo
#endif

#if defined(USART1_IS_USED) || defined(USART2_IS_USED) || defined(USART3_IS_USED) \
    || defined(UART4_IS_USED) || defined(UART5_IS_USED) || defined(USART6_IS_USED) \
    || defined(UART7_IS_USED) || defined(UART8_IS_USED)
    #define UART_IS_USED
#endif

// clang-format on

// ----------------------------------------------------------------------------
#if defined(UART_IS_USED)

// FreeRTOS related configuration
// clang-format off
#ifdef RTOS_USE_FREERTOS
    #ifndef _UART_PREEMPTION_PRIORITY
        #define _UART_PREEMPTION_PRIORITY     6   // cannot be equal or smaller to 5
    #endif
    #ifndef _UART_SUB_PRIORITY
        #define _UART_SUB_PRIORITY            0
    #endif
// ----------------------------------------------------------------------------
// bare-metal
#else
    #ifndef _UART_PREEMPTION_PRIORITY
        #define _UART_PREEMPTION_PRIORITY     10
    #endif
    #ifndef _UART_SUB_PRIORITY
        #define _UART_SUB_PRIORITY            0
    #endif
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(STM32F746xx) || defined(STM32F767xx)
void InitUartPins(GPIO_TypeDef* GPIOx_T, uint8_t pin_nT, GPIO_TypeDef* GPIOx_R,
                  uint8_t pin_nR, uint32_t alter);
#endif  // STM32F746xx || STM32F767xx
void InitUartSettings(UART_HandleTypeDef* huart, uint32_t baud_rate,
                      uint8_t len, char parity, uint8_t stop_b);
void InitUartNvic(IRQn_Type ch, uint16_t p);

// ----------------------------------------------------------------------------

typedef struct {
    void (*config)(uint8_t* data, uint16_t len);
} UartRingBuffer;

typedef struct {
    void (*config)(uint8_t* data, uint32_t len);
} UartDma;

typedef struct {
    UART_HandleTypeDef huart;
    void (*config)(uint32_t, uint8_t, char, uint8_t);
    void (*priority)(uint16_t);
    void (*transmit)(uint8_t*, uint16_t);
    struct RingBuffer_s rb;
    UartDma dma;
    UartRingBuffer ring;
} UartApi_t;

#endif  // UART_IS_USED

// ----------------------------------------------------------------------------
// clang-format off
#if defined(USART1_EXISTS) && defined(USART1_IS_USED)
    extern UartApi_t usart1;
#endif

#if defined(USART2_EXISTS) && defined(USART2_IS_USED)
    extern UartApi_t usart2;
#endif

#if defined(USART3_EXISTS) && defined(USART3_IS_USED)
    extern UartApi_t uart3;
#endif

#if defined(UART4_EXISTS) && defined(UART4_IS_USED)
    extern UartApi_t uart4;
#endif

#if defined(UART5_EXISTS) && defined(UART5_IS_USED)
    extern UartApi_t uart5;
#endif

#if defined(USART6_EXISTS) && defined(USART6_IS_USED)
    extern UartApi_t usart6;
#endif

#if defined(UART7_EXISTS) && defined(UART7_IS_USED)
    extern UartApi_t uart7;
#endif

#if defined(UART8_EXISTS) && defined(UART8_IS_USED)
    extern UartApi_t uart8;
#endif
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __UART_H
