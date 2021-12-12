#ifndef __CONFIG_IIC_H
#define __CONFIG_IIC_H

#include "config.h"

// uint8_t g_config_iic_used;
// clang-format off

// commonly existed ones -------------
#define IIC1_EXISTS
// todo

// special ones ---------------
#if defined(STM32F030x8)
    #define IIC2_EXISTS
#endif

#if defined(STM32F103xB)
    #define IIC2_EXISTS
#endif

#if defined(STM32F107xC)
#endif

#if defined(STM32F303xE)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
#endif

#if defined(STM32F407xx)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
#endif

#if defined(STM32F410Rx)
    #define IIC2_EXISTS
#endif

#if defined(STM32F427xx)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
#endif

#if defined(STM32F446xx)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
#endif

#if defined(STM32F746xx)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
    #define IIC4_EXISTS
#endif

#if defined(STM32F767xx)
    #define IIC2_EXISTS
    #define IIC3_EXISTS
    #define IIC4_EXISTS
#endif

#if defined(IIC1_EXISTS) || defined(IIC2_EXISTS) || defined(IIC3_EXISTS) \
 || defined(IIC4_EXISTS)
    #define IIC_EXISTS
#endif
// clang-format on

#endif  // __CONFIG_IIC_H
