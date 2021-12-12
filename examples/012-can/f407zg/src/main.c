#include "config.h"
#include <math.h>

// =============================================================================
static void taskCan1Test(void) {
    uint8_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = (i + 1) * 16 + (i + 1);
    }
    static uint32_t loop_count = 0;
    if (can1.sendData(0xAA, data, 8) != HAL_OK) {
        console.printf("%5d: failed to send a CAN1 message\r\n", loop_count);
    }
    else {
        console.printf("%5d: sent a CAN1 message\r\n", loop_count);
    }
    loop_count++;
}

// =============================================================================
static void taskCan2Test(void) {
    uint8_t data[8];
    data[0] = 0x88;
    data[1] = 0x99;

    static uint32_t loop_count = 0;
    if (can2.sendData(0xBB, data, 2) != HAL_OK) {
        console.printf("%5d: failed to send a CAN2 message\r\n", loop_count);
    }
    else {
        console.printf("%5d: sent a CAN2 message\r\n", loop_count);
    }
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
    if (!can1.checkBitRate(428)) {
        console.printf("can1 bit rate check failed!\r\n");
    };
    can1.config(1000, CAN_MODE_NORMAL);
    can2.config(1000, CAN_MODE_NORMAL);
    // tasks -----------
    stime.scheduler.attach(2000, 1, taskCan1Test, "taskCan1Test");
    stime.scheduler.attach(2000, 1000, taskCan2Test, "taskCan2Test");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
