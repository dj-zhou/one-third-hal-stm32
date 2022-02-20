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
    uint16_t type = usart2.ring.get.type(msg);
    console.printf("%5d: VelCmdCallback: type = 0x%04X\r\n", loop_count++,
                   type);
    CommVelCmd_t cmd;
    usart2.message.copy(msg, (uint8_t*)&cmd, sizeof(cmd));
    console.printf("\tx_vel = %f, y_vel = %f, yaw_vel = %f\r\n", cmd.x_vel,
                   cmd.y_vel, cmd.yaw_vel);
}

static void SwitchModeCallback(uint8_t* msg) {
    static uint32_t loop_count = 0;
    uint16_t type = usart2.ring.get.type(msg);
    console.printf("%5d: SwitchModeCallback: type = 0x%04X: ", loop_count++,
                   type);
    CommSwitchMode_t switch_mode;
    usart2.message.copy(msg, (uint8_t*)&switch_mode, sizeof(switch_mode));
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
    send_packet((void*)&vel_cmd, sizeof(vel_cmd));
    loop_count++;
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    // usart2 is used for communication
    usart2.config(2000000, 8, 'n', 1);
    uint8_t usart2_rx[100];
    usart2.dma.config(usart2_rx, sizeof_array(usart2_rx));
    uint8_t header[] = { 0xAB, 0xCD, 0xEF };
    usart2.ring.set.header(header, sizeof_array(header));
    usart2.ring.set.length(3, sizeof(uint16_t));
    usart2.ring.set.type(5, sizeof(uint16_t));
    if (!usart2.message.attach(CommVelCmd, VelCmdCallback, "VelCmdCallback")) {
        console.printf("message attach failed\r\n");
    }
    if (!usart2.message.attach(CommSwitchMode, SwitchModeCallback,
                               "SwitchModeCallback")) {
        console.printf("message attach failed\r\n");
    }
    usart2.message.show();

    // tasks -----------
    stime.scheduler.attach(1000, 200, taskSend, "taskSend");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
