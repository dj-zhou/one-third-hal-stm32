#include "spi.h"

// ============================================================================
#if defined(SPI1_EXISTS) && defined(SPI1_IS_USED)

// ============================================================================
#if defined(_USE_SPI1_PA7PA6)
// PA7: MOSI; PA6: MISO; PA5: SCK; PA4: NSS (hardware)
static void InitSpi1_PA7PA6(void) {
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F767xx)
    if (spi1.param.nss == 'h') {
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
#endif  // _USE_SPI1_PA7PA6

// ============================================================================
#if defined(_USE_SPI1_PB5PB4)
static void InitSpi1_PB5PB4(void) {
    // todo
}
#endif  //  _USE_SPI1_PB5PB4

// ----------------------------------------------------------------------------
static void InitSpi1(uint16_t prescale, const char* master,
                     const char* hardware_nss, const char* sck,
                     const char* phase) {
    // g_config_spi_used |= 1 << 1;  // not started from 0
    // -------------------
    if (strcmp(master, "master") == 0) {
        spi1.param.master = 'm';
    }
    else if (strcmp(master, "slave") == 0) {
        spi1.param.master = 's';
    }
    // -------------------
    if (strcmp(hardware_nss, "hard") == 0) {
        spi1.param.nss = 'h';
    }
    else if (strcmp(hardware_nss, "soft") == 0) {
        spi1.param.nss = 'h';
    }
    // -------------------
    if (strcmp(sck, "high") == 0) {
        spi1.param.sck = 'h';
    }
    else if (strcmp(sck, "low") == 0) {
        spi1.param.sck = 'l';
    }
    // -------------------
    if (strcmp(phase, "falling") == 0) {
        spi1.param.sck = 'f';
    }
    else if (strcmp(phase, "rising") == 0) {
        spi1.param.sck = 'r';
    }

    spi1.param.nss_GPIOx = NULL;
    spi1.hspi.Instance   = SPI1;
#if defined(_USE_SPI1_PA7PA6)
    InitSpi1_PA7PA6();
#elif defined(_USE_SPI1_PB5PB4)
    InitSpi1_PB5PB4();
#endif
    InitSpiSettings(&(spi1.hspi), prescale, spi1.param);
}

// ----------------------------------------------------------------------------
static void InitSpi1SoftNss(GPIO_TypeDef* GPIOx_NSS, uint8_t pin_nss) {
    InitSpiSoftNss(&spi1, GPIOx_NSS, pin_nss);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef Spi1Transceive8bits(uint8_t* t_data, uint8_t* r_data,
                                             uint16_t len) {
    uint32_t timeout = 1000;
    return SpiTransceive8bits(&spi1, t_data, r_data, len, timeout);
}
// ----------------------------------------------------------------------------
static HAL_StatusTypeDef Spi1Transceive16bits(uint16_t* t_data,
                                              uint16_t* r_data, uint16_t len) {
    uint32_t timeout = 1000;
    return SpiTransceive16bits(&spi1, t_data, r_data, len, timeout);
}

// ============================================================================
// ---------------------
// clang-format off
SpiApi_t spi1 = {
    .config           = InitSpi1             ,
    .setNss           = InitSpi1SoftNss      ,
    .transceive8bits  = Spi1Transceive8bits  ,
    .transceive16bits = Spi1Transceive16bits ,
};
// clang-format on

// ============================================================================
#endif  // SPI1_EXISTS && SPI1_IS_USED
