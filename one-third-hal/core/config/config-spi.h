#ifndef __CONFIG_SPI_H
#define __CONFIG_SPI_H

#include "config.h"

uint8_t g_config_spi_used;

// clang-format off

// commonly existed ones -------------
#define SPI1_EXISTS
#define SPI2_EXISTS
// todo

// special ones ---------------
#if defined(STM32F107xC)
    #define SPI3_EXISTS
#endif

#if defined(STM32F303xE)
    #define SPI3_EXISTS
#endif

#if defined(STM32F407xx)
    #define SPI3_EXISTS
#endif

#if defined(STM32F427xx)
    #define SPI3_EXISTS
    #define SPI4_EXISTS
#endif

#if defined(STM32F746xx)
    #define SPI3_EXISTS
    #define SPI4_EXISTS
    #define SPI5_EXISTS
    #define SPI6_EXISTS
#endif

#if defined(STM32F767xx)
    #define SPI3_EXISTS
    #define SPI4_EXISTS
    #define SPI5_EXISTS
    #define SPI6_EXISTS
#endif

// clang-format on

#endif  // __CONFIG_SPI_H
