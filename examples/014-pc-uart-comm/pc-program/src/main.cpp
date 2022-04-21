#include "comm.h"
#include "protocol.h"
#include "serial.h"
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;

static void parse_comm_pose_info(const uint8_t* array) {
    CommPoseInfo_t pose_info;
    uint8_t* ptr = (uint8_t*)&pose_info;
    for (size_t i = 0; i < sizeof(pose_info); i++) {
        *ptr++ = array[5 + i];
    }
    printf("receive pose info: x = %f, y = %f, yaw = %f\r\n", pose_info.x,
           pose_info.y, pose_info.yaw);
}

static void parse_comm_battery_info(const uint8_t* array) {
    CommBatteryInfo_t battery_info;
    uint8_t* ptr = (uint8_t*)&battery_info;
    for (size_t i = 0; i < sizeof(battery_info); i++) {
        *ptr++ = array[5 + i];
    }
    printf("receive battery info: soc = %.2f%%, current = %.2fA, voltage = "
           "%.2fV\r\n",
           battery_info.soc * 100, battery_info.current, battery_info.voltage);
}

const auto packet_parse = [](const uint8_t* array, const uint16_t size) {
    CommType_e type = protocol_get_type(array, size);
    uint32_t crc32_get = protocol_get_crc32(array, size);
    uint32_t crc32_cal = protocol_calculate_crc32(array, size);
    if (crc32_get != crc32_cal) {
        printf("CRC32 check failed, get 0x%04X, calculate 0x%04X", crc32_get,
               crc32_cal);
        return;
    }
    switch (type) {
    case CommPoseInfo:
        std::cout << "receive CommPoseInfo message" << std::endl;
        parse_comm_pose_info(array);
        return;
    case CommBatteryInfo:
        std::cout << "receive CommBatteryInfo message" << std::endl;
        parse_comm_battery_info(array);
        return;
    case CommFirmwareInfo:
        std::cout << "receive CommFirmwareInfo message" << std::endl;
        return;
    case CommVelCmd:
    case CommSwitchMode:
        return;
    default:
        break;
    }
    std::cout << "receive unknown message (" << size
              << ") bytes: " << std::endl;
    for (uint16_t i = 0; i < size; i++) {
        printf("%02X ", array[i]);
    }
    std::cout << std::endl;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("usage\n:\t $ %s [serial port]\n", argv[0]);
        return -1;
    }
    Serial serial(argv[1], 2000000, 'n');

    CommVelCmd_t vel_cmd = {
        .type = CommVelCmd,
        .x_vel = (float)0.8,
        .y_vel = (float)0.4,
        .yaw_vel = (float)0.2,
    };
    CommSwitchMode_t switch_mode = {
        .type = CommSwitchMode,
        .mode = 1,
    };

    PcComm pc_comm(&serial, 100, 5, packet_parse);
    for (int loop_count = 0; loop_count < 10; loop_count++) {
        switch_mode.mode = (uint8_t)(loop_count % 256);
        pc_comm.send(switch_mode);
        usleep(10000);
        vel_cmd.x_vel = (float)(0.8 + sin(loop_count / 100.));
        vel_cmd.y_vel = (float)(0.8 + cos(loop_count / 100.));
        vel_cmd.yaw_vel = (float)(0.2 + cos(loop_count / 50.));
        pc_comm.send(vel_cmd);
        sleep(1);
    }
    pc_comm.setStop();
    serial.quit();
    return 0;
}
