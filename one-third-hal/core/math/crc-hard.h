#pragma once

#include "config.h"
#include <stdint.h>

// #if defined(STM32F407xx)
// #include "stm32f4xx_hal_rcc_ex.h"
// #endif

// ============================================================================
typedef struct {
    uint32_t (*_8bit)(uint8_t* buf, uint32_t len, bool reset);
    uint32_t (*_32bit)(uint32_t* buf, uint32_t len);
} Crc32Api_t;
typedef struct {
    CRC_HandleTypeDef hcrc;
    void (*config)(void);
    Crc32Api_t crc32;
} CrcHardApi_t;

// ============================================================================
extern CrcHardApi_t crc_hard;
