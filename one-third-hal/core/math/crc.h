#pragma once

#include "config.h"

// ============================================================================
typedef struct {
    uint16_t (*_16bit8)(uint8_t*, uint32_t, uint8_t);
    uint32_t (*_32bit8)(uint8_t*, uint32_t);
} CrcSoftApi_t;

typedef struct {
    CRC_HandleTypeDef hcrc;
    void (*config)(void);
    uint32_t (*_32bit8)(uint8_t* buf, uint32_t len);
    uint32_t (*_32bit32)(uint32_t* buf, uint32_t len);
} CrcHardApi_t;

typedef struct {
    CrcSoftApi_t soft;
    CrcHardApi_t hard;
} CrcApi_t;

// ============================================================================
extern CrcApi_t crc;
