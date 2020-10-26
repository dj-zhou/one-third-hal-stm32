#ifndef __CORE_UTILS_H
#define __CORE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"  // must be on the top
#include "stm32f1xx.h"

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
#if !defined( _SWAP16 )
#define _SWAP16( x ) ( x = ( uint16_t )( ( x >> 8 ) | ( x << 8 ) ) )
#endif

#if !defined( _SWAP32 )
#define _SWAP32( x )                                  \
    ( x = ( ( x >> 24 ) | ( ( x >> 8 ) & 0x0000ff00 ) \
            | ( ( x << 8 ) & 0x00ff0000 ) | ( x << 24 ) ) )
#endif

#if !defined( _CHECK_BIT )
#define _CHECK_BIT( var, pos ) ( ( var ) & ( 1 << ( pos ) ) )
#endif

#if !defined( _SIZE_OF_ARRAY )
#define _SIZE_OF_ARRAY( x ) ( sizeof( x ) / sizeof( ( x )[0] ) )
#endif

#if !defined( _PACK )
#define _PACK( x ) __attribute__( ( packed, aligned( x ) ) )
#endif

// ============================================================================
// RTOS related macros

// FreeRTOS must use any of these two macros --------------------
// configUSE_TIME_SLICING
// configUSE_PREEMPTION
#if defined( configUSE_TIME_SLICING ) || defined( configUSE_PREEMPTION )
#if !defined( RTOS_USE_FREERTOS )
#define RTOS_USE_FREERTOS
#endif
#endif
// some other RTOS related macros list here

// other RTOS can be added behind
#if defined( RTOS_USE_FREERTOS )
#define RTOS_IS_USED
#endif

#if defined( RTOS_IS_USED )
// this allows for adding other RTOS
typedef enum {
    FREERTOS_NOSTART = ( ( uint8_t )1 ),
    FREERTOS_STARTED = ( ( uint8_t )2 ),
} RtosState_t;
#endif

// ============================================================================
// component API
// clang-format off
typedef struct {
    HAL_StatusTypeDef ( *initSystemClock )( void )                          ;
    void ( *initNvic )( uint8_t group )                                     ;
    void ( *enableGpioClock )( GPIO_TypeDef* GPIOx )                        ;
    void ( *enableTimerClock )( TIM_TypeDef* TIMx )                         ;
    void ( *enableUartClock )( USART_TypeDef* USARTx )                      ;
    void ( *setPinMode )( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t io ) ;
    void ( *setPinPull )( GPIO_TypeDef* GPIOx, uint8_t pin_n, uint32_t p )  ;
    void ( *setPin )( GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v )          ;
    void ( *togglePin )( GPIO_TypeDef* GPIOx, uint8_t pin_n )               ;
#if defined( RTOS_IS_USED )
    void        ( *setRtosState )( RtosState_t state );
    RtosState_t ( *getRtosState )( void );
#endif
} CoreUtilsApi_t;

extern CoreUtilsApi_t utils;
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __CORE_UTILS_H

// ============================================================================
// supported system clock
// MCU-type     HSE_VALUE   SYSCLK    PCLK1     PCLK2     ADCCLK??
// STM32F107xC  25000000    72000000  36000000  72000000  xx
