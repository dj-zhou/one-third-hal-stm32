#include "spi.h"
#include <string.h>

#if defined(SPI_IS_USED)

// ============================================================================
void InitSpiSettings(SPI_HandleTypeDef* hspi, uint16_t prescale,
                     SpiParam_t param) {
    utils.clock.enableSpi(hspi->Instance);
    if (param.master == 'm') {
        hspi->Init.Mode = SPI_MODE_MASTER;
    }
    else {
        hspi->Init.Mode = SPI_MODE_SLAVE;
    }
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    // 16 bits of data is not supported by HAL, so we will fix this option to
    // 8 bits. 16 bits data transmission is however supported by this library
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    if (param.sck == 'h') {
        hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
    }
    else {
        hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
    }
    if (param.phase == 'f') {
        hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
    }
    else if (param.phase == 'r') {
        hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
    }
    else {
        // todo
    }
    if (param.nss == 's') {
        hspi->Init.NSS = SPI_NSS_SOFT;
    }
    else {
        if (param.master == 'm') {
            hspi->Init.NSS = SPI_NSS_HARD_OUTPUT;
        }
        else {
            hspi->Init.Mode = SPI_NSS_HARD_INPUT;  // for slave, just a guess
        }
    }
    switch (prescale) {
    case 2:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
        break;
    case 4:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
        break;
    case 8:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
        break;
    case 16:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
        break;
    case 32:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
        break;
    case 64:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
        break;
    case 128:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
        break;
    case 256:
        hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
        break;
    default:
        console.error("%s(): wrong SPI prescale\r\n", __func__);
        break;
    }
    // not sure if it is on APB2, need test
    // STM32F767 is on PCLK2, tested!
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    console.printk(0, "%s(): SPI baudrate = %3.3f KHz\r\n", __func__,
                   pclk2 / prescale / 1000.0);
    hspi->Init.FirstBit       = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode         = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
#if defined(STM32F407xx) || defined(STM32F427xx)
    hspi->Init.CRCPolynomial = 10;  // as from the CubeMX generated code
#elif defined(STM32F767xx)
    hspi->Init.CRCPolynomial = 7;  // as from the CubeMX generated code
    hspi->Init.CRCLength     = SPI_CRC_LENGTH_DATASIZE;
    hspi->Init.NSSPMode      = SPI_NSS_PULSE_ENABLE;
#else
#error InitSpiSettings(): to do
#endif
    if (HAL_SPI_Init(hspi) != HAL_OK) {
        // Error_Handler();  // TODO
    }
}

// ============================================================================
HAL_StatusTypeDef SpiTransceive8bits(SpiApi_t* spi, uint8_t* tbuf,
                                     uint8_t* rbuf, uint16_t len,
                                     uint32_t timeout) {
    if (spi->param.nss == 's') {
        if (spi->param.nss_GPIOx != NULL) {
            utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 0);
        }
        else {
            console.error("%s(): Soft NSS is not initialized\r\n", __func__);
        }
    }

    stime.delay.us(_SPI_START_TIME_DELAY_US);  // to test
    if (_SPI_END_TIME_DELAY_US == 0) {
        HAL_SPI_TransmitReceive(&(spi->hspi), tbuf, rbuf, len, timeout);
    }
    else {
        for (int i = 0; i < len; i++) {
            HAL_SPI_TransmitReceive(&(spi->hspi), tbuf++, rbuf++, 1, timeout);
            // STM32F767xx shows a 1-2us delay between bytes by default
            stime.delay.us(_SPI_BYTE_TIME_DELAY_US);  // to test
        }
    }
    stime.delay.us(_SPI_END_TIME_DELAY_US);  // to test
    if (spi->param.nss == 's') {
        utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 1);
    }
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef SpiTransceive16bits(SpiApi_t* spi, uint16_t* tbuf,
                                      uint16_t* rbuf, uint16_t len,
                                      uint32_t timeout) {
    if (spi->param.nss == 's') {
        if (spi->param.nss_GPIOx != NULL) {
            utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 0);
        }
        else {
            console.error("%s(): Soft NSS is not initialized\r\n", __func__);
        }
    }

    stime.delay.us(_SPI_START_TIME_DELAY_US);  // to test
    uint16_t d_send;
    uint16_t d_recv;
    uint8_t* d_send_p = ( uint8_t* )&d_send;
    uint8_t* d_recv_p = ( uint8_t* )&d_recv;

    for (int i = 0; i < len; i++) {
        d_send = tbuf[i];
        _SWAP_16(d_send);
        HAL_SPI_TransmitReceive(&(spi->hspi), d_send_p, d_recv_p, 2, timeout);
        _SWAP_16(d_recv);
        rbuf[i] = d_recv;
        stime.delay.us(_SPI_BYTE_TIME_DELAY_US);  // to test
    }
    stime.delay.us(_SPI_END_TIME_DELAY_US);  // to test
    if (spi->param.nss == 's') {
        utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 1);
    }
    return HAL_OK;
}

// ----------------------------------------------------------------------------
void InitSpiSoftNss(SpiApi_t* spi, GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss) {
    spi->param.nss_GPIOx = GPIOx_NSS;
    spi->param.nss_pin   = pin_nss;
    utils.pin.mode(GPIOx_NSS, pin_nss, GPIO_MODE_OUTPUT_PP);
    utils.pin.pull(GPIOx_NSS, pin_nss, GPIO_PULLUP);
    utils.pin.set(GPIOx_NSS, pin_nss, true);
}

// ============================================================================
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F767xx)
void InitSpiPins(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo, GPIO_TypeDef* GPIOx_MI,
                 uint8_t pin_mi, GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                 uint32_t alter) {
    utils.clock.enableGpio(GPIOx_MO);
    utils.clock.enableGpio(GPIOx_MI);
    utils.clock.enableGpio(GPIOx_SCK);

    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // MOSI
    GPIO_InitStructure.Pin       = 1 << pin_mo;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init(GPIOx_MO, &GPIO_InitStructure);
    // MISO
    GPIO_InitStructure.Pin   = 1 << pin_mi;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_MI, &GPIO_InitStructure);
    // SCK
    GPIO_InitStructure.Pin   = 1 << pin_sck;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_SCK, &GPIO_InitStructure);
}

// ----------------------------------------------------------------------------
// not tested
void InitSpiPinsHardNss(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo,
                        GPIO_TypeDef* GPIOx_MI, uint8_t pin_mi,
                        GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                        GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss,
                        uint32_t alter) {
    utils.clock.enableGpio(GPIOx_SCK);
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // NSS
    GPIO_InitStructure.Pin   = 1 << pin_nss;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_NSS, &GPIO_InitStructure);
    InitSpiPins(GPIOx_MO, pin_mo, GPIOx_MI, pin_mi, GPIOx_SCK, pin_sck, alter);
}
#endif  // STM32F407xx || STM32F427xx || STM32F767xx

// ============================================================================
#endif  // SPI_IS_USED
