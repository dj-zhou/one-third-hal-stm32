#include "crc-hard.h"

// ============================================================================
static void CrcResetValue(void) {
    CRC->CR = 1;
}

// ============================================================================
static void CrcHardConfig(void) {
    crc_hard.hcrc.Instance = CRC;
    if (HAL_CRC_Init(&crc_hard.hcrc) != HAL_OK) {
        // Error_Handler();
    }
    __HAL_RCC_CRC_CLK_ENABLE();
    CrcResetValue();
}

// ============================================================================
static uint32_t CrcHard8bitCalculate(uint8_t* buf, uint32_t len, bool reset) {
    if (reset) {
        CrcResetValue();
    }
    for (uint32_t i = 0; i < len; i++) {
        CRC->DR = ( uint32_t )buf[i];
    }
    return (CRC->DR);
}

// ============================================================================
static uint32_t CrcHard32bitCalculate(uint32_t* buf, uint32_t len) {
    CrcResetValue();
    for (uint32_t i = 0; i < len; i++) {
        CRC->DR = buf[i];
    }
    return (CRC->DR);
}

// ============================================================================
// variable
CrcHardApi_t crc_hard = {
    .config = CrcHardConfig,
    .crc32 = {
        ._8bit = CrcHard8bitCalculate,
        ._32bit = CrcHard32bitCalculate,
    },
};
