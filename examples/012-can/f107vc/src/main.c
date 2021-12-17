#include "config.h"

// =============================================================================
static void taskCan1Test(void) {
    uint8_t data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = (i + 1) * 16 + (i + 1);
    }

    uint16_t id = 0x555;
    uint8_t len = 8;
    static uint32_t loop_count = 0;
    if (can1.sendData(id, data, len) != HAL_OK) {
        console.printf("%5d: failed to send a CAN1 message\r\n", loop_count);
    }
    else {
        console.printf("%5d: sent a CAN1 message: (0x%04X, %d)", loop_count, id,
                       len);
        for (int i = 0; i < len; i++) {
            console.printf(" %02X", data[i]);
        }
        console.printf("\r\n");
    }
    loop_count++;
}

// ============================================================================
static void Can1IrqTest(CAN_RxHeaderTypeDef* msg, uint8_t* data) {
    console.printf(GRN "%s: " NOC, __func__);
    for (int i = 0; i < msg->DLC; i++) {
        console.printf(" 0x%02X", data[i]);
    }
    console.printf("\r\n");
}

// ============================================================================
int main(void) {
    utils.system.initClock(72, 36, 72);
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

    can1.irq.attach(0x666, Can1IrqTest, "Can1IrqTest");
    // tasks -----------
    stime.scheduler.attach(2000, 1, taskCan1Test, "taskCan1Test");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}