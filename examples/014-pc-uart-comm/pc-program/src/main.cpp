#include "comm.h"
#include "protocol.h"
#include "serial.h"
#include <iostream>
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

    PcComm pc_comm(&serial);
    usleep(10000);
    pc_comm.send(vel_cmd);
    usleep(10000);
    pc_comm.send(switch_mode);
    sleep(1);
    // for (int i = 0; i < 10; i++) {
    //     const char* str = "hello world\n";
    //     pc_comm.send(( char* )str, sizeof(str));
    //     sleep(1);
    // }
    serial.quit();
    return 0;
}
