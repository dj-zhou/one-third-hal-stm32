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
//  3     10    len, low 8 bits
//  4     00    len, high 8 bits, so here len = 0x13 = 19, means next byte to
//              end has 19 bytes in total (type + payload + CRC16 + tail)
//  5     XX    type, low 8 bits
//  6     XX    type, high 8 bits
//     ------------  from here, is the payload of a message type
//  7     XX
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
// 18
//     ------------ above here, is the payload of a message type
// 19    XX     CRC16, low 8 bits
// 20    XX     CRC16, high 8 bits
// 21    FE     tail[0]
// 22    DC     tail[1]
// 23    BA     tail[2]

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

void comm_serialize(const char* data, size_t data_size, char* packet);

#ifdef __cplusplus
}
#endif
