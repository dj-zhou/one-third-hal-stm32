#ifndef __UART_CONSOLE_H
#define __UART_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif
// ============================================================================

// if use this library, we must we this module
#include "config.h"

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// project interface --------------
// clang-format off
#if defined(_CONSOLE_USE_USART1_PA9PA10) || defined(_CONSOLE_USE_USART1_PB6PB7)
    #define CONSOLE_USE_USART1
#endif

#if defined(_CONSOLE_USE_USART2_PA2PA3) || defined(_CONSOLE_USE_USART2_PD5PD6)
    #define CONSOLE_USE_USART2
#endif

#if defined(_CONSOLE_USE_USART3_PB10PB11) || defined(_CONSOLE_USE_USART3_PC10PC11) \
   || defined (_CONSOLE_USE_USART3_PD8PD9)
    #define CONSOLE_USE_USART3
#endif

#if defined(_CONSOLE_USE_UART4_PC10PC11)
    #define CONSOLE_USE_UART4
#endif

#if defined(_CONSOLE_USE_UART5_PC12PD2)
    #define CONSOLE_USE_UART5
#endif

// USART6: TODO

#if defined(_CONSOLE_USE_UART7_PE8PE7)
    #define CONSOLE_USE_UART7
#endif

#if defined(CONSOLE_USE_USART1)     \
    || defined(CONSOLE_USE_USART2)  \
    || defined(CONSOLE_USE_USART3)  \
    || defined(CONSOLE_USE_UART4)   \
    || defined(CONSOLE_USE_UART5)   \
    || defined(CONSOLE_USE_USART6)  \
    || defined(CONSOLE_USE_UART7)
    #define CONSOLE_IS_USED
#endif
// clang-format on

// ============================================================================
#if defined(CONSOLE_IS_USED)

// clang-format off
#if !defined(_CONSOLE_SIGN_DATA_SIZE)
    #define    _CONSOLE_SIGN_DATA_SIZE    (50)  // > 20
#endif

#if !defined(_CONSOLE_BUFF_LEN)
    #define    _CONSOLE_BUFF_LEN         (100)
#endif

// priority ----------------------------
#if defined(RTOS_USE_FREERTOS)
    #if !defined(_CONSOLE_PREEMPTION_PRIORITY)
        #define _CONSOLE_PREEMPTION_PRIORITY        20
    #endif
    #if !defined(_CONSOLE_SUB_PRIORITY)
        #define _CONSOLE_SUB_PRIORITY                0
    #endif
// #elif defined ( XXXX RTOS )
// bare-metal
#else
    #if !defined(_CONSOLE_PREEMPTION_PRIORITY)
        #define _CONSOLE_PREEMPTION_PRIORITY        20
    #endif
    #if !defined(_CONSOLE_SUB_PRIORITY)
        #define _CONSOLE_SUB_PRIORITY                0
    #endif
#endif
// clang-format on

// ============================================================================
typedef HAL_StatusTypeDef (*CliHandle)(int argc, char** argv);

// larger number means less important log
typedef enum {
    LOG_CRIT = 0,  // Critical messages, cannot be shut off
    LOG_WARNING,   // Warning conditions that should be taken care of.
    LOG_INFO,  // informational messages that require no action, can be shut off
} LogLevel_e;

typedef enum {
    TX_PP = 0,
    TX_OD = 1,
} UartTx_e;

typedef struct {
    void (*mode)(UartTx_e);
} ConsoleTx;

typedef struct {
    void (*enable)(bool);
    bool (*get)(void);
    void (*setStatus)(bool);
    bool (*getStatus)(void);
} ConsoleRx;

typedef struct {
    void (*byte)(char);
    void (*str)(char*);
} ConsoleWrite;

typedef struct {
    void (*setLevel)(LogLevel_e l);
    void (*attach)(char* str, CliHandle p);
    void (*process)(void);
} ConsoleCli;

// ============================================================================
// clang-format off
typedef struct {
    LogLevel_e level;
    void (*config)(uint32_t)                           ;
    void (*printk)(LogLevel_e level, char* format, ...);
    void (*printf)(char* format, ...)                  ;
    void (*error)(char* format, ...)                   ;
    char (*read)(uint16_t)                             ;
    ConsoleTx    tx    ;
    ConsoleRx    rx    ;
    ConsoleWrite write ;
    ConsoleCli   cli   ;
} Console_t;
extern Console_t console;
// clang-format on

#endif  // CONSOLE_IS_USED

#if defined(CONSOLE_IS_USED)
#define CONSOLE_PRINTF_SEG                                                    \
    do {                                                                      \
        console.printk(0,                                                     \
                       YLW "------------------------------------------------" \
                           "--------------------------------\r\n" NOC);       \
    } while (0)
#else
#define CONSOLE_PRINTF_SEG \
    do {                   \
        ;                  \
    } while (0)
#endif

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __UART_CONSOLE_H
