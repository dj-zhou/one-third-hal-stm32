#include "config.h"

#include "comm.h"
#include "protocol.h"
#include <math.h>

uint8_t usart1_rx[100];

CommBatteryInfo_t battery_info = {
    .type = CommBatteryInfo,
    .voltage = (float)11.2,
    .current = (float)0.3,
    .soc = (float)0.78,
};

CommVelCmd_t vel_cmd = {
    .type = CommVelCmd,
    .x_vel = (float)0.8,
    .y_vel = (float)0.4,
    .yaw_vel = (float)0.2,
};
// ============================================================================
void Usart1IdleIrq(void) {
    usart1.ring.show('h', 10);
}

// ============================================================================
int main(void) {
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    console.printf("sizeof(CommVelCmd_t) = %ld\r\n", sizeof(CommVelCmd_t));
    console.printf("sizeof(CommSwitchMode_t) = %ld\r\n",
                   sizeof(CommSwitchMode_t));
    console.printf("sizeof(CommPoseInfo_t) = %ld\r\n", sizeof(CommPoseInfo_t));
    console.printf("sizeof(CommBatteryInfo_t) = %ld\r\n",
                   sizeof(CommBatteryInfo_t));
    console.printf("sizeof(CommFirmwareInfo_t) = %ld\r\n",
                   sizeof(CommFirmwareInfo_t));

    // usart1 is used for communication
    usart1.config(2000000, 8, 'n', 1);
    usart1.dma.config(usart1_rx, sizeof_array(usart1_rx));

    // ----------------
    send_packet((void*)&battery_info, sizeof(battery_info));
    stime.delay.ms(100);
    // (test only) ----------------
    send_packet((void*)&vel_cmd, sizeof(vel_cmd));
    stime.delay.ms(100);

    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
