#pragma once

// ============================================================================
#include "config.h"

#include "config-uart.h"
#include "general.h"
#include "operation.h"
#include <string.h>

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
    #if defined(_USE_UART4_PA0PA1) || defined(_USE_UART4_PC10PC11)
        #define UART4_IS_USED
    #endif
#endif

#if defined(UART5_EXISTS)
    #if defined(_USE_UART5_PC12PD2)
        #define UART5_IS_USED
    #endif
#endif

#if defined(USART6_EXISTS)
    #if defined(_USE_USART6_PC6PC7) || defined(_USE_USART6_PG14PG9)
        #define USART6_IS_USED
    #endif
#endif


#if defined(UART7_EXISTS)
    #if defined(_USE_UART7_PA15PA8) || defined(_USE_UART7_PB4PB3)
        #define UART7_IS_USED
    #endif
#endif

#if defined(UART8_EXISTS)
    #if defined(_USE_UART8_PE1PE0) || defined(_USE_UART8_PJ8PJ9)
        #define UART8_IS_USED
    #endif
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
        #define _UART_PREEMPTION_PRIORITY    10
    #endif
    #ifndef _UART_SUB_PRIORITY
        #define _UART_SUB_PRIORITY            0
    #endif
#endif

// ----------------------------------------------------------------------------
#ifndef _UART_MESSAGE_NODE_MAX_NUM
    #define _UART_MESSAGE_NODE_MAX_NUM       50
#endif
#ifndef _UART_MESSAGE_DESCR_SIZE
    #define _UART_MESSAGE_DESCR_SIZE         50
#endif
#ifndef _UART_MESSAGE_MAX_PACKET_SIZE
    #define _UART_MESSAGE_MAX_PACKET_SIZE   100
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(STM32F746xx) || defined(STM32F767xx)
void init_uart_pins(GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                    GPIO_TypeDef* GPIOx_R, uint8_t pin_nR, uint32_t alter);
#endif  // STM32F746xx || STM32F767xx
void init_uart_settings(UART_HandleTypeDef* huart, uint32_t baud_rate,
                        uint8_t len, char parity, uint8_t stop_b);
void init_uart_nvic(IRQn_Type ch, uint16_t p);

// ----------------------------------------------------------------------------
typedef struct {
    bool is_used;
    void (*config)(uint8_t* data, uint16_t len);
} UartDma_t;

typedef struct {
    void (*config)(uint8_t* data, uint16_t len);
    void (*show)(char style, uint16_t width);
    WARN_UNUSED_RESULT int8_t (*search)(void);
    WARN_UNUSED_RESULT int8_t (*fetch)(uint8_t* array, uint16_t size);
} UartRingBuffer_t;

typedef void (*usart_irq_hook)(uint8_t* msg);

// clang-format off
typedef struct UartMessageCpnt_s {
    uint16_t     msg_type;
    char         descr[_UART_MESSAGE_DESCR_SIZE];
    usart_irq_hook hook;
} UartMessageCpnt_t;

typedef struct UartMessageNode_s {
    struct UartMessageCpnt_s  this_;
    struct UartMessageNode_s* next_;
} UartMessageNode_t;

typedef struct {
    uint8_t header[RINGBUFFER_HEADER_MAX_LEN];
    uint8_t header_len;
    uint8_t len_pos;
    uint8_t len_width;
    uint8_t type_pos;
    uint8_t type_width;
}UartMessageInfo_t;

typedef struct {
    void (*header)(uint8_t* data, uint8_t len);
    void (*length)(uint8_t pos, uint8_t width);
    void (*type)(uint8_t pos, uint8_t width);
} UartMessageSet_t;

typedef struct {
    uint16_t (*length)(uint8_t *data);
    uint16_t (*type)(uint8_t *data);
} UartMessageGet_t;

typedef struct {
    bool (*attach)(uint16_t type, usart_irq_hook hook, const char * descr);
    void (*show)(void);
    void (*copy)(uint8_t*msg, uint8_t*dest, size_t size);
    UartMessageSet_t set;
    UartMessageGet_t get;
} UartMessage_t;
// clang-format on

void uart_message_set_header(uint8_t* header, uint8_t len,
                             UartMessageInfo_t* msg_info);
void uart_message_set_length(uint8_t pos, uint8_t width,
                             UartMessageInfo_t* msg_info);
uint16_t uart_message_get_length(uint8_t* data, UartMessageInfo_t* msg_info);
void uart_message_set_type(uint8_t pos, uint8_t width,
                           UartMessageInfo_t* msg_info);
uint16_t uart_message_get_type(uint8_t* data, UartMessageInfo_t* msg_info);
bool uart_message_attach(uint16_t type, usart_irq_hook hook, const char* descr,
                         UartMessageNode_t* node, uint8_t node_num);
void uart_message_show(const char* port, UartMessageNode_t* node,
                       uint8_t node_num);
// clang-format off
typedef struct {
    UART_HandleTypeDef huart;
    void (*config)(uint32_t, uint8_t, char, uint8_t);
    void (*priority)(uint16_t);
    void (*send)(uint8_t*, uint16_t);
    UartDma_t        dma;
    RingBuffer_t     rb;
    UartRingBuffer_t ring;
    UartMessage_t    message;
} UartApi_t;
// clang-format on

#ifdef CONSOLE_IS_USED
#define uart_printf(...) (console.printf(__VA_ARGS__))
#define uart_printk(...) (console.printk(__VA_ARGS__))
#define uart_error(...) (console.error(__VA_ARGS__))
#else
#define uart_printf(...) ({ ; })
#define uart_printk(...) ({ ; })
#define uart_error(...) ({ ; })
#endif

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

#endif  // UART_IS_USED
