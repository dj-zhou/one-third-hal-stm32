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
    static uint32_t loop_count = 0;
    uint16_t type = usart1.ring.get.type(msg);
    console.printf("%5d: VelCmdCallback: type = 0x%04X\r\n", loop_count++,
                   type);
    CommVelCmd_t cmd;
    usart1.message.copy(msg, (uint8_t*)&cmd, sizeof(cmd));
    console.printf("\tx_vel = %f, y_vel = %f, yaw_vel = %f\r\n", cmd.x_vel,
                   cmd.y_vel, cmd.yaw_vel);
}

static void SwitchModeCallback(uint8_t* msg) {
    static uint32_t loop_count = 0;
    uint16_t type = usart1.ring.get.type(msg);
    console.printf("%5d: SwitchModeCallback: type = 0x%04X: ", loop_count++,
                   type);
    CommSwitchMode_t switch_mode;
    usart1.message.copy(msg, (uint8_t*)&switch_mode, sizeof(switch_mode));
    console.printf("mode = %d\r\n", switch_mode.mode);
}

// ============================================================================
void taskSend(void) {
    static uint32_t loop_count = 0;
    // ----------------
    battery_info.voltage = (float)(10.5 + sin(loop_count / 100.));
    send_packet((void*)&battery_info, sizeof(battery_info));
    stime.delay.ms(100);
    // (test only) ----------------
    vel_cmd.x_vel = (float)(0.8 + sin(loop_count / 100.));
    vel_cmd.y_vel = (float)(0.8 + cos(loop_count / 100.));
    vel_cmd.yaw_vel = (float)(0.2 + cos(loop_count / 50.));
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

    // usart1 is used for communication
    usart1.config(2000000, 8, 'n', 1);
    uint8_t usart1_rx[100];
    usart1.dma.config(usart1_rx, sizeof_array(usart1_rx));
    uint8_t header[] = { 0xAB, 0xCD, 0xEF };
    usart1.ring.set.header(header, sizeof_array(header));
    usart1.ring.set.length(3, sizeof(uint16_t));
    usart1.ring.set.type(5, sizeof(uint16_t));
    if (!usart1.message.attach(CommVelCmd, VelCmdCallback, "VelCmdCallback")) {
        console.printf("message attach failed\r\n");
    }
    if (!usart1.message.attach(CommSwitchMode, SwitchModeCallback,
                               "SwitchModeCallback")) {
        console.printf("message attach failed\r\n");
    }
    usart1.message.show();

    console.printf("sizeof(CommVelCmd_t) = %d\r\n", sizeof(CommVelCmd_t));
    console.printf("sizeof(CommBatteryInfo_t) = %d\r\n",
                   sizeof(CommBatteryInfo_t));

    // tasks -----------
    stime.scheduler.attach(1000, 200, taskSend, "taskSend");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
