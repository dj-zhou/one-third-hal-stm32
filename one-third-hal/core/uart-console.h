#ifndef __UART_CONSOLE_H
#define __UART_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

// if use this library, we must we this module
#include "config.h"

#include <stdbool.h>
#include <stdint.h>

#define CONSOLE_PRINTF_SEG                                                     \
    do {                                                                       \
        console.printk(                                                        \
            0, YLW "----------------------------------------------\r\n" NOC ); \
    } while ( 0 )

// ============================================================================
// project interface --------------
// clang-format off

#if !defined( _CONSOLE_USE_USART1_PA9PA10 )  \
 && !defined( _CONSOLE_USE_USART2_PA2PA3 )   \
 && !defined( _CONSOLE_USE_USART2_PD5PD6 )   \
 && !defined( _CONSOLE_USE_USART3_PB10PB11 ) \
 && !defined( _CONSOLE_USE_UART4 )           \
 && !defined( _CONSOLE_USE_UART5_PC12PD2 )
    #define _CONSOLE_USE_USART2_PA2PA3 // stm32f103 does not have UART5
#endif

#if defined( _CONSOLE_USE_USART1_PA9PA10 )  \
 || defined( _CONSOLE_USE_USART2_PA2PA3 )   \
 || defined( _CONSOLE_USE_USART2_PD5PD6 )   \
 || defined( _CONSOLE_USE_USART3_PB10PB11 ) \
 || defined( _CONSOLE_USE_UART4 )           \
 || defined( _CONSOLE_USE_UART5_PC12PD2 )
    #define CONSOLE_IS_USED
#endif

#if !defined( _CONSOLE_SIGN_DATA_SIZE )
    #define _CONSOLE_SIGN_DATA_SIZE     ( 50 ) // > 20
#endif

#if !defined( _CONSOLE_BUFF_LEN )
    #define _CONSOLE_BUFF_LEN           ( 100 )
#endif

// clang-format on

// ============================================================================
#if defined( CONSOLE_IS_USED )
typedef enum {
    TX_PP = 0,
    TX_OD = 1,
} ConsoleTx_e;

// to print with color (escaping)
#define NOC "\033[0m"
#define GRY "\033[0;30m"
#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define YLW "\033[0;33m"
#define BLU "\033[0;34m"
#define PRP "\033[0;35m"
#define CYN "\033[0;36m"
#define WHT "\033[0;37m"
#define HGRY "\033[1;30m"
#define HRED "\033[1;31m"
#define HGRN "\033[1;32m"
#define HYLW "\033[1;33m"
#define HBLU "\033[1;34m"
#define HPRP "\033[1;35m"
#define HCYN "\033[1;36m"
#define HWHT "\033[1;37m"

// priority ----------------------------
// clang-format off
#if defined( RTOS_USE_FREERTOS )
    #if !defined( _CONSOLE_PREEMPTION_PRIORITY)
        #define _CONSOLE_PREEMPTION_PRIORITY     20
    #endif
    #if !defined( _CONSOLE_SUB_PRIORITY)
        #define _CONSOLE_SUB_PRIORITY             0
    #endif
// #elif defined ( XXXX RTOS )
// bare-metal
#else
    #if !defined( _CONSOLE_PREEMPTION_PRIORITY)
        #define _CONSOLE_PREEMPTION_PRIORITY     20
    #endif
    #if !defined( _CONSOLE_SUB_PRIORITY)
        #define _CONSOLE_SUB_PRIORITY             0
    #endif
#endif
// clang-format on

// ============================================================================
typedef HAL_StatusTypeDef ( *CliHandle )( int argc, char** argv );

// larger number means less important log
typedef enum {
    LOG_CRIT = 0,  // Critical messages, cannot be shut off
    LOG_WARNING,   // Warning conditions that should be taken care of.
    LOG_INFO,  // informational messages that require no action, can be shut off
} LogLevel_e;

typedef struct {
    void ( *setLevel )( LogLevel_e l );
    void ( *regist )( char* str, CliHandle p );
    void ( *process )( void );
} Cli;
// ============================================================================
// clang-format off
typedef struct {
    LogLevel_e level                                         ;
    void   ( *config )( uint32_t, uint8_t, char, uint8_t )   ;
    void   ( *setTxMode )( ConsoleTx_e )                     ;
    void   ( *enableRxen )( bool )                           ;
    void   ( *printk )( LogLevel_e level, char* format, ... );
    void   ( *printf )( char* format, ... )                  ;
    void   ( *error )( char* format, ... )                   ;
    void   ( *writeByte )( char )                            ;
    void   ( *writeStr )( char* )                            ;
    char   ( *read )( uint16_t )                             ;
    // command line interface
    Cli     cli  ;
} Console_t;
extern Console_t console;
// clang-format on

#endif  // CONSOLE_IS_USED

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __UART_CONSOLE_H
