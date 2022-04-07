#include "comm.h"
#include "protocol.h"
#include "serial.h"
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;
const auto packet_parse = [](const uint8_t* array, const uint16_t size) {
    CommType_e type = protocol_get_type(array, size);
    switch (type) {
    case CommVelCmd:
        std::cout << "receive CommVelCmd message" << std::endl;
        return;
    case CommSwitchMode:
        std::cout << "receive CommSwitchMode message" << std::endl;
        return;
    case CommPoseInfo:
        std::cout << "receive CommPoseInfo message" << std::endl;
        return;
    case CommBatteryInfo:
        std::cout << "receive CommBatteryInfo message" << std::endl;
        return;
    case CommFirmwareInfo:
        std::cout << "receive CommFirmwareInfo message" << std::endl;
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
