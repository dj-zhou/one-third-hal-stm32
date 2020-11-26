
#ifndef __CRC_SOFT_H
#define __CRC_SOFT_H

#include "config.h"

// ============================================================================
typedef struct {
    uint16_t (*calculate8bitCrc16)(uint8_t*, uint16_t, uint8_t);
    uint32_t (*calculate8bitCrc32)(uint8_t*, uint16_t);
    uint32_t (*calculate32bitCrc32)(uint32_t*, uint16_t);
} CrcSoftApi_t;

// ============================================================================
extern CrcSoftApi_t crc_soft;

// ============================================================================

#endif  // __CRC_SOFT_H
