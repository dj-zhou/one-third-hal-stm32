#include "spi.h"

// ============================================================================
#if defined(SPI_IS_USED)
static void InitSpiSettings(SPI_HandleTypeDef* hspi, uint16_t prescale,
                            bool master, bool hardware_nss) {
    utils.clock.enableSpi(hspi->Instance);
    if (master) {
        hspi->Init.Mode = SPI_MODE_MASTER;
    }
    else {
        hspi->Init.Mode = SPI_MODE_SLAVE;
    }
    hspi->Init.Direction   = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize    = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi->Init.CLKPhase    = SPI_PHASE_1EDGE;
    if (!hardware_nss) {
        hspi->Init.NSS = SPI_NSS_SOFT;
    }
    else {
        if (master) {
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
#if defined(STM32F427xx)
    hspi->Init.CRCPolynomial = 10;
#elif defined(STM32F767xx)
    hspi->Init.CRCPolynomial = 7;
    hspi->Init.CRCLength     = SPI_CRC_LENGTH_DATASIZE;
    hspi->Init.NSSPMode      = SPI_NSS_PULSE_ENABLE;
#else
#error todo
#endif
    if (HAL_SPI_Init(hspi) != HAL_OK) {
        // Error_Handler();  // TODO
    }
}

// ============================================================================
HAL_StatusTypeDef SpiTransceive(SpiApi_t* spi, uint8_t* tbuf, uint8_t* rbuf,
                                uint16_t len, uint32_t timeout) {
    if (spi->param.nss_GPIOx == NULL) {
        console.error("%s(): Soft NSS not initialized\r\n", __func__);
    }
    if (spi->param.nss == SPI_SOFT_NSS) {
        utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 0);
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
    if (spi->param.nss == SPI_SOFT_NSS) {
        utils.pin.set(spi->param.nss_GPIOx, spi->param.nss_pin, 1);
    }
    return HAL_OK;
}

// ----------------------------------------------------------------------------
static void InitSpiSoftNss(SpiApi_t* spi, GPIO_TypeDef* GPIOx_NSS,
                           uint8_t pin_nss) {
    spi->param.nss_GPIOx = GPIOx_NSS;
    spi->param.nss_pin   = pin_nss;
    utils.pin.mode(GPIOx_NSS, pin_nss, GPIO_MODE_OUTPUT_PP);
    utils.pin.set(GPIOx_NSS, pin_nss, true);  // initial state: high
}

// ============================================================================
#if defined(STM32F427xx) || defined(STM32F767xx)
static void InitSpiPins(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo,
                        GPIO_TypeDef* GPIOx_MI, uint8_t pin_mi,
                        GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                        uint32_t alter) {
    utils.clock.enableGpio(GPIOx_MO);
    utils.clock.enableGpio(GPIOx_MI);
    utils.clock.enableGpio(GPIOx_SCK);

    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // MOSI
    GPIO_InitStructure.Pin       = 1 << pin_mo;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init(GPIOx_MO, &GPIO_InitStructure);
    // MISO
    GPIO_InitStructure.Pin   = 1 << pin_mi;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_MI, &GPIO_InitStructure);
    // SCK
    GPIO_InitStructure.Pin   = 1 << pin_sck;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_SCK, &GPIO_InitStructure);
}

// ----------------------------------------------------------------------------
// not tested
static void InitSpiPinsHardNss(GPIO_TypeDef* GPIOx_MO, uint8_t pin_mo,
                               GPIO_TypeDef* GPIOx_MI, uint8_t pin_mi,
                               GPIO_TypeDef* GPIOx_SCK, uint8_t pin_sck,
                               GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss,
                               uint32_t alter) {
    utils.clock.enableGpio(GPIOx_SCK);
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // NSS
    GPIO_InitStructure.Pin   = 1 << pin_nss;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_NSS, &GPIO_InitStructure);
    InitSpiPins(GPIOx_MO, pin_mo, GPIOx_MI, pin_mi, GPIOx_SCK, pin_sck, alter);
}
#endif  // STM32F427xx

// ============================================================================
#if defined(SPI1_EXISTS) && defined(_USE_SPI1_PA7PA6)
static void InitSpi1_PA7PA6(void) {

#if defined(STM32F427xx) || defined(STM32F767xx)
    if (spi1.param.nss == SPI_HARD_NSS) {
        InitSpiPinsHardNss(GPIOA, 7, GPIOA, 6, GPIOA, 5, GPIOA, 4,
                           GPIO_AF5_SPI1);
    }
    else {
        InitSpiPins(GPIOA, 7, GPIOA, 6, GPIOA, 5, GPIO_AF5_SPI1);
        // initialize the software nss pin by another function call
    }
#else
#error InitSpi1_PA7PA6(): to implement and verify!
#endif
}
#endif  // SPI1_EXISTS || _USE_SPI1_PA7PA6

// ============================================================================
#if defined(SPI1_EXISTS) && defined(_USE_SPI1_PB5PB4)
static void InitSpi1_PB5PB4(void) {
    // todo
}
#endif  // SPI1_EXISTS || _USE_SPI1_PB5PB4

// ============================================================================
#if defined(SPI1_EXISTS) && defined(SPI1_IS_USED)

// ----------------------------------------------------------------------------
static void InitSpi1(uint16_t prescale, SpiMaster_e master,
                     SpiNss_e hardware_nss) {
    g_config_spi_used |= 1 << 1;  // not started from 0
    spi1.param.master    = master;
    spi1.param.nss       = hardware_nss;
    spi1.param.nss_GPIOx = NULL;
    spi1.hspi.Instance   = SPI1;
#if defined(_USE_SPI1_PA7PA6)
    InitSpi1_PA7PA6();
#elif defined(_USE_SPI1_PB5PB4)
    InitSpi1_PB5PB4();
#endif
    InitSpiSettings(&(spi1.hspi), prescale, master, hardware_nss);
}

// ----------------------------------------------------------------------------
static void InitSpi1SoftNss(GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss) {
    InitSpiSoftNss(&spi1, GPIOx_NSS, pin_nss);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef Spi1Transceive(uint8_t* t_data, uint8_t* r_data,
                                        uint16_t len) {
    uint32_t timeout = 1000;
    return SpiTransceive(&spi1, t_data, r_data, len, timeout);
}

// ============================================================================
// ---------------------
// clang-format off
SpiApi_t spi1 = {
    .config       = InitSpi1       ,
    .setNss       = InitSpi1SoftNss,
    .transceive   = Spi1Transceive ,
};
// clang-format on
#endif  // SPI1_EXISTS && SPI1_IS_USED

// ============================================================================
#endif  // SPI_IS_USED
