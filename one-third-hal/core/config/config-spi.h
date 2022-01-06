#pragma once

#include "config.h"
#include <stdbool.h>

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

#if defined(STM32F410Rx)
    #define SPI5_EXISTS
#endif

#if defined(STM32F427xx)
    #define SPI3_EXISTS
    #define SPI4_EXISTS
#endif

#if defined(STM32F446xx)
    #define SPI3_EXISTS
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

#if defined(STM32H750xx)
    #define SPI3_EXISTS
    #define SPI4_EXISTS
    #define SPI5_EXISTS
    #define SPI6_EXISTS
#endif

#if defined(SPI1_EXISTS) || defined(SPI2_EXISTS) || defined(SPI3_EXISTS) \
 || defined(SPI4_EXISTS) || defined(SPI5_EXISTS) || defined(SPI6_EXISTS)
    #define SPI_EXISTS
#endif
// clang-format on

typedef struct {
    void (*set)(SPI_TypeDef* SPIx, bool value);
    bool (*check)(SPI_TypeDef* SPIx);
} ConfigSpi_t;

extern ConfigSpi_t config_spi;
