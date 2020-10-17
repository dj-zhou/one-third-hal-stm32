#ifndef __CORE_UTILS_H
#define __CORE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "stm32f1xx.h"

// ============================================================================
// clang-format off
typedef struct {
    HAL_StatusTypeDef ( *initSystemClock )( void )                  ;
    void              ( *initNvic )( uint8_t group )                ;
    void              ( *enableGpioClock )( GPIO_TypeDef* GPIOx )   ;
    void              ( *enableTimerClock )( TIM_TypeDef* TIMx )    ;
    void              ( *enableUartClock )( USART_TypeDef* USARTx ) ;
    void              ( *setPinMode )( GPIO_TypeDef* GPIOx, uint8_t pin_n, \
                                        uint32_t io )               ;
    void              ( *setPinPull )( GPIO_TypeDef* GPIOx, uint8_t pin_n, \
                                        uint32_t p )                ;
    void              ( *setPin )( GPIO_TypeDef* GPIOx, uint8_t pin_n, bool v ) ;
    void              ( *togglePin )( GPIO_TypeDef* GPIOx, uint8_t pin_n )      ;

} CoreUtilsApi_t;

extern CoreUtilsApi_t utils;
// clang-format on

#ifdef __cplusplus
}
#endif

#endif  // __CORE_UTILS_H

// ============================================================================
// supported system clock
// MCU-type     HSE_VALUE   SYSCLK    PCLK1     PCLK2     ADCCLK??
// STM32F107xC  25000000    72000000  36000000  72000000  xx
