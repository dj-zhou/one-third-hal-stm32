#include "iic.h"

// ============================================================================
#if defined( IIC_IS_USED )

// ============================================================================
#if defined( IIC1_EXISTS ) && defined( _USE_IIC1_PB8PB9 )
// PB8: I2C1_SCL;  PB9: I2C1_SDA
// tested on F407ZG
// ----------------------------------------------------------------------------
#if defined( STM32F107xC )
static void InitIic1_PB8PB9( void ) {
    console.error( "PB8 and PB9 just do not work for IIC\r\n" );
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    __HAL_AFIO_REMAP_I2C1_ENABLE();
}
#endif  // STM32F107xC

// ----------------------------------------------------------------------------
#if defined( STM32F407xx )
static void InitIic1_PB8PB9( void ) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
}
#endif  // STM32F407xx

#endif  // IIC1_EXISTS || _USE_IIC1_PB8PB9

// ============================================================================
#if defined( IIC1_EXISTS ) && defined( IIC1_IS_USED )

// ----------------------------------------------------------------------------
static void InitIic1( uint32_t speed ) {
    g_config_iic_used |= 1 << 1;  // not started from 0
    iic1.bit_rate = speed;
#if defined( _USE_IIC1_PB6PB7 )
    InitIic1_PB6PB7();  // todo
#elif defined( _USE_IIC1_PB8PB9 )
    InitIic1_PB8PB9();
#elif defined( _USE_IIC1_PB6PB9 )
    InitIic1_PB6PB9();  // todo
#endif
    utils.clock.enableIic( I2C1 );
    iic1.hiic.Instance             = I2C1;
    iic1.hiic.Init.ClockSpeed      = speed;
    iic1.hiic.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    iic1.hiic.Init.OwnAddress1     = 0;
    iic1.hiic.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    iic1.hiic.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    iic1.hiic.Init.OwnAddress2     = 0;
    iic1.hiic.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    iic1.hiic.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if ( HAL_I2C_Init( &( iic1.hiic ) ) != HAL_OK ) {
        console.error( "iic1 initialization failed\r\n" );
    }
    __HAL_I2C_ENABLE( &( iic1.hiic ) );
}

// ----------------------------------------------------------------------------
HAL_StatusTypeDef Iic1_Read( uint16_t dev_addr, uint16_t mem_addr,
                             uint16_t mem_add_size, uint8_t* p_data,
                             uint16_t size, uint32_t timeout ) {
    HAL_StatusTypeDef ret =
        HAL_I2C_Mem_Read( &( iic1.hiic ), dev_addr, mem_addr, mem_add_size,
                          p_data, size, timeout );
    return ret;
}

// ----------------------------------------------------------------------------
HAL_StatusTypeDef Iic1_Write( uint16_t dev_addr, uint16_t mem_addr,
                              uint16_t mem_add_size, uint8_t* p_data,
                              uint16_t size, uint32_t timeout ) {
    HAL_StatusTypeDef ret =
        HAL_I2C_Mem_Write( &( iic1.hiic ), dev_addr, mem_addr, mem_add_size,
                           p_data, size, timeout );
    return ret;
}
// ----------------------------------------------------------------------------
// clang-format off
IicApi_t iic1 = {
    .config = InitIic1  ,
    .read   = Iic1_Read ,
    .write  = Iic1_Write,
};
// clang-format on
#endif  // IIC1_EXISTS && IIC1_IS_USED

// ============================================================================
#endif  // IIC_IS_USED
