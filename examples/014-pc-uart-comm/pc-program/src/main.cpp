#include "comm.h"
#include "protocol.h"
#include "serial.h"
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("usage\n:\t $ %s [serial port]\n", argv[0]);
        return -1;
    }
    Serial serial(argv[1], 2000000, 'n');

    printf("sizeof(CommVelCmd_t) = %ld\n", sizeof(CommVelCmd_t));
    printf("sizeof(CommSwitchMode_t) = %ld\n", sizeof(CommSwitchMode_t));
    printf("sizeof(CommPoseInfo_t) = %ld\n", sizeof(CommPoseInfo_t));
    printf("sizeof(CommBatteryInfo_t) = %ld\n", sizeof(CommBatteryInfo_t));
    printf("sizeof(CommFirmwareInfo_t) = %ld\n", sizeof(CommFirmwareInfo_t));

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

    PcComm pc_comm(&serial, 100, 5);
    uint32_t loop_count = 0;
    for (int i = 0; i < 1; i++) {
        pc_comm.send(switch_mode);
        usleep(10000);
        vel_cmd.x_vel = (float)(0.8 + sin(loop_count / 100));
        vel_cmd.y_vel = (float)(0.8 + cos(loop_count / 100));
        pc_comm.send(vel_cmd);
        sleep(1);
    }
    pc_comm.setStop();
    serial.quit();
    return 0;
}
