#pragma once

#include "config.h"

// clang-format off
// commonly existed ones -------------
#define GPIOA_EXISTS
#define GPIOB_EXISTS
#define GPIOC_EXISTS
#define GPIOD_EXISTS

// special ones ---------------
#if defined(STM32F107xC)
    #define GPIOE_EXISTS
#endif

#if defined(STM32F303xE)
    #define GPIOE_EXISTS  // not sure
    #define GPIOF_EXISTS
#endif

#if defined(STM32F410Rx)
    #define GPIOH_EXISTS
#endif

#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    #define GPIOE_EXISTS
    #define GPIOF_EXISTS
    #define GPIOG_EXISTS
    #define GPIOH_EXISTS
    #define GPIOI_EXISTS
#endif

#if defined(STM32H750xx)
    #define GPIOE_EXISTS
    #define GPIOF_EXISTS
    #define GPIOG_EXISTS
    #define GPIOH_EXISTS
    #define GPIOI_EXISTS
    #define GPIOJ_EXISTS
#endif

// clang-format on
