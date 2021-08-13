#ifndef __IIC_EEPROM_H
#define __IIC_EEPROM_H

#include "config.h"

#include "iic.h"
#include <stdint.h>

// ============================================================================
// clang-format off
#if defined(_EEPROM_USE_AT24C08)  // || other eeprom
    #define EEPROM_IS_USED

#if defined(_AT24C08_ON_IIC1)
    #define EEPROM_IIC iic1
#endif
#if defined(_AT24C08_ON_IIC2)
    #define EEPROM_IIC iic2
#endif
#if defined(_AT24C08_ON_IIC3)
    #define EEPROM_IIC iic3
#endif

#if !defined(_AT24C08_ON_IIC1) && !defined(_AT24C08_ON_IIC2) \
    && !defined(_AT24C08_ON_IIC3)
    #error iic-eeprom: eeprom device should be on one of the iic ports!
#endif
#endif
// clang-format on

// ============================================================================
// note:
// this file only works for 24cxx EEPROM chip, and only tested with
// 24LC08BT-I/OT (SOT-23-5)

// ============================================================================
// EEPROM 24LC08BT-I/OT has 1024 bytes of space, device address is (0xA0 | (0x01
// << 3)) = 0xA8
//      device address       offset address
//  block 0    0xA8                 0 - 255
//  block 1    0xAA                 0 - 255
//  block 2    0xAC                 0 - 255
//  block 3    0xAE                 0 - 255
// however, the chip only support for reading 16 bytes (in one block) at once

// ============================================================================
//  similar devices and their storage space (bytes)
// EEPROM_AT24C01           128
// EEPROM_AT24C02           256
// EEPROM_AT24C04           512
// EEPROM_AT24C08          1024
// EEPROM_AT24C16          2048
// EEPROM_AT24C32          4096
// EEPROM_AT24C64          8192
// EEPROM_AT24C128        16384
// EEPROM_AT24C256        32768

// ============================================================================
// clang-format off

// ----------------------------------------------------------------------------
// chip specific macros
#if defined(_EEPROM_USE_AT24C08)
    #define EEPROM_TOTAL_BYTE_NUM   1024
    #define EEPROM_BLOCK_SIZE        256
    #define EEPROM_PAGE_SIZE          16
    #define EEPROM_FIRST_BLOCK_ADDR  (0xA0 | (0x01 << 3))
    #define EEPROM_BLOCK_ADDR_DIFF  0x02  // not used??
#endif
// clang-format on

// ============================================================================
// clang-format off
#if !defined(_EEPROM_NODE_MAX_NUM)
    #define _EEPROM_NODE_MAX_NUM        10
#endif

#if !defined(_EEPROM_NODE_START_ADDR)
    #define _EEPROM_NODE_START_ADDR      0
#endif

#if !defined(_EEPROM_NODE_MAX_READ_TIME)
    #define _EEPROM_NODE_MAX_READ_TIME   5
#endif
// clang-format on

// ============================================================================
#if defined(EEPROM_IS_USED)

// ============================================================================
#define EEPROM_KEY_01 0xFA01
#define EEPROM_KEY_02 0xFA02
#define EEPROM_KEY_03 0xFA03
#define EEPROM_KEY_04 0xFA04
#define EEPROM_KEY_05 0xFA05
#define EEPROM_KEY_06 0xFA06
#define EEPROM_KEY_07 0xFA07
#define EEPROM_KEY_08 0xFA08
#define EEPROM_KEY_09 0xFA09
#define EEPROM_KEY_10 0xFA10
#define EEPROM_KEY_11 0xFA11
#define EEPROM_KEY_12 0xFA12
#define EEPROM_KEY_13 0xFA13
#define EEPROM_KEY_14 0xFA14
#define EEPROM_KEY_15 0xFA15
#define EEPROM_KEY_16 0xFA16
#define EEPROM_KEY_17 0xFA17
#define EEPROM_KEY_18 0xFA18
#define EEPROM_KEY_19 0xFA19
#define EEPROM_KEY_20 0xFA20
#define EEPROM_KEY_21 0xFA21
#define EEPROM_KEY_22 0xFA22
#define EEPROM_KEY_23 0xFA23
#define EEPROM_KEY_24 0xFA24
#define EEPROM_KEY_25 0xFA25
#define EEPROM_KEY_26 0xFA26
#define EEPROM_KEY_27 0xFA27
#define EEPROM_KEY_28 0xFA28
#define EEPROM_KEY_29 0xFA29
#define EEPROM_KEY_30 0xFA30
#define EEPROM_KEY_31 0xFA31
#define EEPROM_KEY_32 0xFA32
#define EEPROM_KEY_33 0xFA33
#define EEPROM_KEY_34 0xFA34
#define EEPROM_KEY_35 0xFA35
#define EEPROM_KEY_36 0xFA36
#define EEPROM_KEY_37 0xFA37
#define EEPROM_KEY_38 0xFA38
#define EEPROM_KEY_39 0xFA39
#define EEPROM_KEY_40 0xFA40
#define EEPROM_KEY_41 0xFA41
#define EEPROM_KEY_42 0xFA42
#define EEPROM_KEY_43 0xFA43
#define EEPROM_KEY_44 0xFA44
#define EEPROM_KEY_45 0xFA45
#define EEPROM_KEY_46 0xFA46
#define EEPROM_KEY_47 0xFA47
#define EEPROM_KEY_48 0xFA48
#define EEPROM_KEY_49 0xFA49
#define EEPROM_KEY_50 0xFA50

#define IS_EEPROM_KEY(key)                                   \
    (((key) == EEPROM_KEY_01) || ((key) == EEPROM_KEY_02)    \
     || ((key) == EEPROM_KEY_03) || ((key) == EEPROM_KEY_04) \
     || ((key) == EEPROM_KEY_05) || ((key) == EEPROM_KEY_06) \
     || ((key) == EEPROM_KEY_07) || ((key) == EEPROM_KEY_08) \
     || ((key) == EEPROM_KEY_09) || ((key) == EEPROM_KEY_10) \
     || ((key) == EEPROM_KEY_11) || ((key) == EEPROM_KEY_12) \
     || ((key) == EEPROM_KEY_13) || ((key) == EEPROM_KEY_14) \
     || ((key) == EEPROM_KEY_15) || ((key) == EEPROM_KEY_16) \
     || ((key) == EEPROM_KEY_17) || ((key) == EEPROM_KEY_18) \
     || ((key) == EEPROM_KEY_19) || ((key) == EEPROM_KEY_20) \
     || ((key) == EEPROM_KEY_21) || ((key) == EEPROM_KEY_22) \
     || ((key) == EEPROM_KEY_23) || ((key) == EEPROM_KEY_24) \
     || ((key) == EEPROM_KEY_25) || ((key) == EEPROM_KEY_26) \
     || ((key) == EEPROM_KEY_27) || ((key) == EEPROM_KEY_28) \
     || ((key) == EEPROM_KEY_29) || ((key) == EEPROM_KEY_30) \
     || ((key) == EEPROM_KEY_31) || ((key) == EEPROM_KEY_32) \
     || ((key) == EEPROM_KEY_33) || ((key) == EEPROM_KEY_34) \
     || ((key) == EEPROM_KEY_35) || ((key) == EEPROM_KEY_36) \
     || ((key) == EEPROM_KEY_37) || ((key) == EEPROM_KEY_38) \
     || ((key) == EEPROM_KEY_39) || ((key) == EEPROM_KEY_40) \
     || ((key) == EEPROM_KEY_41) || ((key) == EEPROM_KEY_42) \
     || ((key) == EEPROM_KEY_43) || ((key) == EEPROM_KEY_44) \
     || ((key) == EEPROM_KEY_45) || ((key) == EEPROM_KEY_46) \
     || ((key) == EEPROM_KEY_47) || ((key) == EEPROM_KEY_48) \
     || ((key) == EEPROM_KEY_49) || ((key) == EEPROM_KEY_50))

typedef struct {
    uint8_t (*byte)(uint16_t);
    void (*bytes)(uint16_t, uint8_t*, uint16_t);
} EepromRead;

typedef struct {
    void (*byte)(uint16_t, uint8_t);
    void (*bytes)(uint16_t, uint8_t*, uint16_t);
} EepromWrite;

typedef struct {
    void (*attach)(uint8_t*, uint16_t);
    void (*read)(uint8_t*, uint16_t);
    void (*write)(uint8_t*, uint16_t);
    void (*show)(void);
} EepromNodeOp;

// ============================================================================
// clang-format off
typedef struct {
    void (*config)(void);
    EepromRead      read;
    EepromWrite    write;
    EepromNodeOp    node;
} EepromApi_t;
// clang-format on
extern EepromApi_t eeprom;

// ============================================================================

#endif  // EEPROM_IS_USED

#endif  // __IIC_EEPROM_H
