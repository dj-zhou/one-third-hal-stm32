#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// =============================================================================
// communication protocol description
// a packet example
// No.   data   desc.
//  0     AB    header[0]
//  1     CD    header[1]
//  2     EF    header[2]
//     ------------
//  3     14    len, low 8 bits
//  4     00    len, high 8 bits, len = payload + CRC32
//     ------------
//  5     XX    payload: type, low 8 bits
//  6     XX    payload: type, high 8 bits
//  7     XX    payload: field 2, low 8 bits ...
//  8
//  9
// 10
// 11
// 12
// 13
// 14
// 15
// 16
// 17
// 18    XX    above here, is the payload of a message type
//     ------------
// 19    XX     CRC32, low 8 bits
// 20    XX     CRC32,
// 21    XX     CRC32,
// 22    XX     CRC32, high 8 bits
//              CRC32 is calculated from header to above CRC

// total size of a packet = len + 5

// =============================================================================
#pragma pack(1)
typedef enum CommType {
    // from PC to F407ZG
    CommVelCmd = 0x0101,
    CommSwitchMode,
    // from F407ZG to PC
    CommPoseInfo = 0x0201,
    CommBatteryInfo,
    CommFirmwareInfo,
} CommType_e;

typedef struct {
    uint16_t type;
    float x_vel;
    float y_vel;
    float yaw_vel;
} CommVelCmd_t;

// need to define mode in enum
typedef struct {
    uint16_t type;
    uint8_t mode;
} CommSwitchMode_t;

typedef struct {
    uint16_t type;
    float x;
    float y;
    float yaw;
} CommPoseInfo_t;

typedef struct {
    uint16_t type;
    float voltage;  // V
    float current;  // A
    float soc;      // 0 ~ 1
} CommBatteryInfo_t;

typedef struct {
    uint16_t type;
    uint8_t version[3];  // version[0]: patch version
    // version[1]: minor version
    // version[2]: major version
    char commit[7];
    char dirty;
} CommFirmwareInfo_t;
#pragma pack()

void protocol_serialize(const char* data, size_t data_size, uint8_t* packet);
uint16_t protocol_get_length(const uint8_t* data, uint16_t data_size);
CommType_e protocol_get_type(const uint8_t* data, uint16_t data_size);
uint32_t protocol_get_crc32(const uint8_t* data, uint16_t data_size);
uint32_t protocol_calculate_crc32(const uint8_t* data, uint16_t data_size);

#ifdef __cplusplus
}
#endif
