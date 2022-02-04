#include "config.h"

#include "comm.h"
#include "protocol.h"
#include <math.h>
#include <stdint.h>

// ============================================================================
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

// ----------------------------------------------------------------------------
static void VelCmdCallback(uint8_t* msg) {
    uint8_t* ptr_msg = msg + 5;
    uint16_t type;
    uint8_t* ptr_type = (uint8_t*)&type;
    *ptr_type++ = *ptr_msg++;
    *ptr_type = *ptr_msg;
    console.printf("VelCmdCallback: type = 0x%04X\r\n", type);
}

static void SwitchModeCallback(uint8_t* msg) {
    uint8_t* ptr_msg = msg + 5;
    uint16_t type;
    uint8_t* ptr_type = (uint8_t*)&type;
    *ptr_type++ = *ptr_msg++;
    *ptr_type = *ptr_msg;
    console.printf("SwitchModeCallback: type = 0x%04X\r\n", type);
}

// ============================================================================
void taskSend(void) {
    static uint32_t loop_count = 0;
    // ----------------
    battery_info.voltage = (float)(10.5 + sin(loop_count / 100.));
    send_packet((void*)&battery_info, sizeof(battery_info));
    stime.delay.ms(100);
    // (test only) ----------------
    send_packet((void*)&vel_cmd, sizeof(vel_cmd));
    loop_count++;
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
    uint8_t usart1_rx[100];
    usart1.dma.config(usart1_rx, sizeof_array(usart1_rx));
    uint8_t header[] = { 0xAB, 0xCD, 0xEF };
    usart1.ring.set.header(header, sizeof_array(header));
    usart1.ring.set.length(3, 2);
    if (!usart1.message.attach(CommVelCmd, VelCmdCallback)) {
        console.printf("failed?\r\n");
    }
    if (!usart1.message.attach(CommSwitchMode, SwitchModeCallback)) {
        console.printf("failedddddd?\r\n");
    }

    // tasks -----------
    stime.scheduler.attach(1000, 200, taskSend, "taskSend");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
