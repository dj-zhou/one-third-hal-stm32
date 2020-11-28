#ifndef __CONFIG_CAN_H
#define __CONFIG_CAN_H

#include "config.h"

uint8_t g_config_can_used;
// clang-format off

// commonly existed ones -------------


// special ones ---------------
#if defined(STM32F030x8)
    // no can exists
#endif

#if defined(STM32F103xB)
    #define CAN1_EXISTS // may just called CAN
#endif

#if defined(STM32F107xC)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
#endif

#if defined(STM32F303xE)
    #define CAN1_EXISTS // may just called CAN
#endif

#if defined(STM32F407xx)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
#endif

#if defined(STM32F410Rx)
    // no can exists
#endif

#if defined(STM32F427xx)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
#endif

#if defined(STM32F446xx)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
#endif

#if defined(STM32F746xx)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
#endif

#if defined(STM32F767xx)
    #define CAN1_EXISTS
    #define CAN2_EXISTS
    #define CAN3_EXISTS
#endif

// clang-format on

#endif  // __CONFIG_CAN_H
