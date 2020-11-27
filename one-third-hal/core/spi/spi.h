#ifndef __SPI_H
#define __SPI_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
#include "config.h"

#include "config-spi.h"

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

#if !defined(_SPI_START_TIME_DELAY_US)
    #define _SPI_START_TIME_DELAY_US 0
#endif

#if !defined(_SPI_BYTE_TIME_DELAY_US)
    #define _SPI_BYTE_TIME_DELAY_US 0
#endif

#if !defined(_SPI_END_TIME_DELAY_US)
    #define _SPI_END_TIME_DELAY_US 0
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(SPI_IS_USED)

typedef enum {
    SPI_MASTER = (( uint8_t )1),
    SPI_SLAVE  = (( uint8_t )2),
} SpiMaster_e;
typedef enum {
    SPI_SOFT_NSS = (( uint8_t )1),
    SPI_HARD_NSS = (( uint8_t )2),
} SpiNss_e;

typedef struct {
    SpiMaster_e   master;
    SpiNss_e      nss;
    GPIO_TypeDef* nss_GPIOx;
    uint8_t       nss_pin;
} SpiParam_t;
typedef struct {
    SPI_HandleTypeDef hspi;
    SpiParam_t        param;
    void (*config)(uint16_t, SpiMaster_e, SpiNss_e);
    void (*setNss)(GPIO_TypeDef*, uint8_t);
    HAL_StatusTypeDef (*transceive)(uint8_t*, uint8_t*, uint16_t);
} SpiApi_t;

#endif  // SPI_IS_USED

// ----------------------------------------------------------------------------
// clang-format off
#if defined( SPI1_EXISTS ) && defined( SPI1_IS_USED )
    extern SpiApi_t spi1;
#endif

#if defined( SPI2_EXISTS ) && defined( SPI2_IS_USED )
    extern SpiApi_t spi2;
#endif

#if defined( SPI3_EXISTS ) && defined( SPI3_IS_USED )
    extern SpiApi_t spi3;
#endif

#if defined( SPI4_EXISTS ) && defined( SPI4_IS_USED )
    extern SpiApi_t spi4;
#endif

#if defined( SPI5_EXISTS ) && defined( SPI5_IS_USED )
    extern SpiApi_t spi5;
#endif

#if defined( SPI6_EXISTS ) && defined( SPI6_IS_USED )
    extern SpiApi_t spi6;
#endif
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __SPI_H
