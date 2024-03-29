#pragma once

#include "config.h"

// clang-format off
// commonly existed ones -------------
#define TIM1_EXISTS
// special ones ---------------
#if defined(STM32F030x8)
    #define TIM3_EXISTS
    #define TIM6_EXISTS
    #define TIM14_EXISTS
    #define TIM15_EXISTS
    #define TIM16_EXISTS
    #define TIM17_EXISTS
#endif

#if defined(STM32F103xB)
    #define TIM2_EXISTS
    #define TIM3_EXISTS
    #define TIM4_EXISTS
#endif

#if defined(STM32F107xC)
    #define TIM2_EXISTS
    #define TIM3_EXISTS
    #define TIM4_EXISTS
    #define TIM5_EXISTS
    #define TIM6_EXISTS
    #define TIM7_EXISTS
#endif

#if defined(STM32F303xE)
    #define TIM2_EXISTS
    #define TIM3_EXISTS
    #define TIM4_EXISTS
    #define TIM6_EXISTS
    #define TIM7_EXISTS
    #define TIM8_EXISTS
    #define TIM15_EXISTS
    #define TIM16_EXISTS
    #define TIM17_EXISTS
#endif

#if defined(STM32F410Rx)
    #define TIM5_EXISTS
    #define TIM6_EXISTS
    #define TIM9_EXISTS
    #define TIM11_EXISTS
#endif

#if defined(STM32F446xx)
    #define TIM5_EXISTS
    #define TIM6_EXISTS
    #define TIM9_EXISTS
    #define TIM11_EXISTS
#endif

#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F446xx) \
  || defined(STM32F746xx) || defined(STM32F767xx)
    #define TIM2_EXISTS
    #define TIM3_EXISTS
    #define TIM4_EXISTS
    #define TIM5_EXISTS
    #define TIM6_EXISTS
    #define TIM7_EXISTS
    #define TIM8_EXISTS
    #define TIM9_EXISTS
    #define TIM10_EXISTS
    #define TIM11_EXISTS
    #define TIM12_EXISTS
    #define TIM13_EXISTS
    #define TIM14_EXISTS
#endif

#if defined(STM32H750xx)
    #define TIM2_EXISTS
    #define TIM3_EXISTS
    #define TIM4_EXISTS
    #define TIM5_EXISTS
    #define TIM6_EXISTS
    #define TIM7_EXISTS
    #define TIM8_EXISTS
    #define TIM12_EXISTS
    #define TIM13_EXISTS
    #define TIM14_EXISTS
    #define TIM15_EXISTS
    #define TIM16_EXISTS
    #define TIM17_EXISTS
    // it also has LPTIM1, LPTIM2, LPTIM3, LPTIM4 and LPTIM5
#endif

// clang-format on
