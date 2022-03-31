#include "comm.h"
#include "protocol.h"
#include "serial.h"
#include <iostream>
#include <math.h>
#include <unistd.h>

using namespace std;
const auto packet_parse = [](const uint8_t* array, const uint16_t size) {
    // todo: use actual parse functions for each CommType
    std::cout << "receive message (" << size << ") bytes: " << std::endl;
    for (uint16_t i = 0; i < size; i++) {
        printf("%3d ", array[i]);
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
