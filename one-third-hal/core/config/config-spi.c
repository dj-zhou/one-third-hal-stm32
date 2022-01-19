#include "config-spi.h"

static uint8_t spi_used = 0;

static uint8_t get_pos(SPI_TypeDef* SPIx) {
    uint8_t pos = 0;
    // clang-format off
    switch ((intptr_t)SPIx) {
    case (intptr_t)SPI1: pos = 0; break;
    case (intptr_t)SPI2: pos = 1; break;
#if defined(SPI3_EXISTS)
    case (intptr_t)SPI3: pos = 2; break;
#endif
#if defined(SPI4_EXISTS)
    case  (intptr_t)SPI4: pos = 3; break;
#endif
#if defined(SPI5_EXISTS)
    case  (intptr_t)SPI5: pos = 4; break;
#endif
#if defined(SPI6_EXISTS)
    case (intptr_t)SPI6: pos = 5; break;
#endif

    default: pos = 0; break;}
    // clang-format on
    return pos;
}

static void ConfigSpiSet(SPI_TypeDef* SPIx, bool value) {
    uint8_t pos = get_pos(SPIx);
    value ? (spi_used |= (uint8_t)(1 << pos))
          : (spi_used &= (uint8_t)(~(1 << pos)));
}

static bool ConfigUSpiCheck(SPI_TypeDef* SPIx) {
    return _CHECK_BIT(spi_used, get_pos(SPIx));
}

ConfigSpi_t config_spi = {
    .set = ConfigSpiSet,
    .check = ConfigUSpiCheck,
};
