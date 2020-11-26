#ifndef __IIC_H
#define __IIC_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
#include "config.h"

#include "config-iic.h"

// ============================================================================

// ============================================================================
// clang-format off
// ----------------------------------------------------------------------------
#if defined( IIC1_EXISTS )
    #if defined( _USE_IIC1_PB6PB7 ) \
     || defined( _USE_IIC1_PB8PB9 ) \
     || defined( _USE_IIC1_PB6PB9 )
        #define IIC1_IS_USED
    #endif
#endif

#if defined( IIC2_EXISTS )
    #if defined( _USE_IIC2_PB10PB11 ) \
     || defined( _USE_IIC2_PF1PF0 )   \
     || defined( _USE_IIC2_PH4PH5 )
        #define IIC2_IS_USED
    #endif
#endif

#if defined( IIC3_EXISTS )
    #if defined( _USE_IIC3_PA8PC9 ) \
     || defined( _USE_IIC3_PH7PH8 )
        #define IIC3_IS_USED
    #endif
#endif

#if defined( IIC4_EXISTS )
    #if defined( _USE_IIC4_PD12PD13 ) \
     || defined( _USE_IIC4_PF1PF0 )   \
     || defined( _USE_IIC4_PF14PF15 ) \
     || defined( _USE_IIC4_PH11PH12 )
        #define IIC4_IS_USED
    #endif
#endif



#if defined( IIC1_IS_USED ) || defined( IIC2_IS_USED ) \
 || defined( IIC3_IS_USED ) || defined( IIC4_IS_USED )
    #define IIC_IS_USED
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(IIC_IS_USED)

typedef struct {
    uint32_t          bit_rate;
    I2C_HandleTypeDef hiic;
    void (*config)(uint32_t);
    HAL_StatusTypeDef (*read)(uint16_t, uint16_t, uint16_t, uint8_t*, uint16_t,
                              uint32_t);
    HAL_StatusTypeDef (*write)(uint16_t, uint16_t, uint16_t, uint8_t*, uint16_t,
                               uint32_t);
} IicApi_t;

#endif  // IIC_IS_USED

// clang-format off
// ----------------------------------------------------------------------------
#if defined( IIC1_EXISTS ) && defined( IIC1_IS_USED )
    extern IicApi_t iic1;
#endif

#if defined( IIC2_EXISTS ) && defined( IIC2_IS_USED )
    extern IicApi_t iic2;
#endif

#if defined( IIC3_EXISTS ) && defined( IIC3_IS_USED )
    extern IicApi_t iic3;
#endif

#if defined( IIC4_EXISTS ) && defined( IIC4_IS_USED )
    extern IicApi_t iic4;
#endif

// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __IIC_H
