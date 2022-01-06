#pragma once

// ============================================================================
#include "config.h"

#include "config-spi.h"
#include <string.h>

// ============================================================================
// https://stm32f4-discovery.net/2014/04/library-05-spi-for-stm32f4xx/
// example:
//   _USE_SPI1_PA7PA6: PA7 is the MOSI pin, and PA6 is the MISO pin
//                      you can find the SCK pin by the above link

// ============================================================================
// clang-format off
// ----------------------------------------------------------------------------
#if defined(SPI1_EXISTS)
    #if defined(_USE_SPI1_PA7PA6) || defined(_USE_SPI1_PB5PB4)
        #define SPI1_IS_USED
    #endif
#endif

#if defined(SPI2_EXISTS)
    #if defined(_USE_SPI2_PC3PC2) || defined(_USE_SPI2_PB15PB14)
        #define SPI2_IS_USED
    #endif
#endif

#if defined(SPI3_EXISTS)
    #if defined(_USE_SPI3_PB5PB4) || defined(_USE_SPI3_PC12PC11)
        #define SPI3_IS_USED
    #endif
#endif

#if defined(SPI4_EXISTS)
    #if defined(_USE_SPI4_PE6PE5) || defined(_USE_SPI4_PE14PE13)
        #define SPI4_IS_USED
    #endif
#endif

#if defined(SPI5_EXISTS)
    #if defined(_USE_SPI5_PF9PF8) || defined(_USE_SPI5_PF11PH7)
        #define SPI5_IS_USED
    #endif
#endif

#if defined(SPI6_EXISTS)
    #if defined(_USE_SPI6_PG14PG12)
        #define SPI6_IS_USED
    #endif
#endif

#if defined(SPI1_IS_USED) || defined(SPI2_IS_USED) || defined(SPI3_IS_USED) \
    || defined(SPI4_IS_USED) || defined(SPI5_IS_USED) || defined(SPI6_IS_USED)
    #define SPI_IS_USED
#endif

// different devices need different settings -- to fix -----------
#if !defined(_SPI_START_TIME_DELAY_US)
    #define _SPI_START_TIME_DELAY_US    0
#endif

#if !defined(_SPI_BYTE_TIME_DELAY_US)
    #define _SPI_BYTE_TIME_DELAY_US     0
#endif

#if !defined(_SPI_END_TIME_DELAY_US)
    #define _SPI_END_TIME_DELAY_US      0
#endif

// clang-format on

// ----------------------------------------------------------------------------
#if defined(SPI_IS_USED)
typedef struct {
    unsigned char master;  // 'm' or 's'
    unsigned char nss;     // 'h' or 's'
    unsigned char sck;     // 'h' or 'l'
    unsigned char phase;   // 'f' or 'r'
    GPIO_TypeDef* nss_GPIOx;
    uint8_t nss_pin;
} SpiParam_t;

typedef struct {
    SPI_HandleTypeDef hspi;
    SpiParam_t param;
    void (*config)(uint16_t, const char*, const char*, const char*,
                   const char*);
    void (*setNss)(GPIO_TypeDef*, uint8_t);
    HAL_StatusTypeDef (*transceive8bits)(uint8_t*, uint8_t*, uint16_t);
    HAL_StatusTypeDef (*transceive16bits)(uint16_t*, uint16_t*, uint16_t);
} SpiApi_t;

void InitSpiSettings(SPI_HandleTypeDef* hspi, uint16_t prescale,
                     SpiParam_t param);
HAL_StatusTypeDef SpiTransceive8bits(SpiApi_t* spi, uint8_t* tbuf,
                                     uint8_t* rbuf, uint16_t len,
                                     uint32_t timeout);
HAL_StatusTypeDef SpiTransceive16bits(SpiApi_t* spi, uint16_t* tbuf,
                                      uint16_t* rbuf, uint16_t len,
                                      uint32_t timeout);
void InitSpiSoftNss(SpiApi_t* spi, GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss);

#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F767xx)
void InitSpiPins(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo, GPIO_TypeDef* GPIOx_MI,
                 uint8_t pin_mi, GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                 uint32_t alter);
void InitSpiPinsHardNss(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo,
                        GPIO_TypeDef* GPIOx_MI, uint8_t pin_mi,
                        GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                        GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss,
                        uint32_t alter);
#endif  // STM32F407xx || STM32F427xx || STM32F767xx

#ifdef CONSOLE_IS_USED
#define spi_printf(...) (console.printf(__VA_ARGS__))
#define spi_printk(...) (console.printk(__VA_ARGS__))
#define spi_error(...) (console.error(__VA_ARGS__))
#else
#define spi_printf(...) ({ ; })
#define spi_printk(...) ({ ; })
#define spi_error(...) ({ ; })
#endif

// ----------------------------------------------------------------------------
// clang-format off
#if defined(SPI1_EXISTS) && defined(SPI1_IS_USED)
    extern SpiApi_t spi1;
#endif

#if defined(SPI2_EXISTS) && defined(SPI2_IS_USED)
    extern SpiApi_t spi2;
#endif

#if defined(SPI3_EXISTS) && defined(SPI3_IS_USED)
    extern SpiApi_t spi3;
#endif

#if defined(SPI4_EXISTS) && defined(SPI4_IS_USED)
    extern SpiApi_t spi4;
#endif

#if defined(SPI5_EXISTS) && defined(SPI5_IS_USED)
    extern SpiApi_t spi5;
#endif

#if defined(SPI6_EXISTS) && defined(SPI6_IS_USED)
    extern SpiApi_t spi6;
#endif
// clang-format on

#endif  // SPI_IS_USED
