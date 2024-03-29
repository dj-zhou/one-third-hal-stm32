#include "config.h"

// =============================================================================
static void taskCan1Test1(void) {
    uint8_t data[8];
    for (uint8_t i = 0; i < 8; i++) {
        data[i] = (uint8_t)((i + 1) * 16 + (i + 1));
    }

    uint16_t id = 0x666;
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

// =============================================================================
static void taskCan1Test2(void) {
    uint8_t data[8];
    for (uint8_t i = 0; i < 8; i++) {
        data[i] = (uint8_t)((i + 1) * 8 + (i + 1));
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

// =============================================================================
static void taskCan2Test(void) {
    uint8_t data[8];
    data[0] = 0x88;
    data[1] = 0x99;
    data[2] = 0xAA;
    data[3] = 0xBB;

    uint16_t id = 0x0666;
    uint8_t len = 4;
    static uint32_t loop_count = 0;
    if (can2.sendData(id, data, len) != HAL_OK) {
        console.printf("%5d: failed to send a CAN2 message\r\n", loop_count);
    }
    else {
        console.printf("%5d: sent a CAN2 message: (0x%04X, %d)", loop_count, id,
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
static void Can2IrqTest(CAN_RxHeaderTypeDef* msg, uint8_t* data) {
    console.printf(GRN "%s: " NOC, __func__);
    for (int i = 0; i < msg->DLC; i++) {
        console.printf(" 0x%02X", data[i]);
    }
    console.printf("\r\n");
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

    // can -----------
    if (!can1.checkBitRate(428)) {
        console.printf("can1 bit rate check failed!\r\n");
    };
    can1.config(1000, CAN_MODE_NORMAL);
    can2.config(1000, CAN_MODE_NORMAL);
    can1.irq.attach(0x666, Can1IrqTest, "Can1IrqTest");
    can1.irq.attach(0x666, Can2IrqTest, "Can2IrqTest");  // this will fail
    can2.irq.attach(0x0AB, Can2IrqTest, "Can2IrqTest");
    can1.irq.show();
    can2.irq.show();

    // tasks -----------
    stime.scheduler.attach(2000, 1, taskCan1Test1, "taskCan1Test1");
    stime.scheduler.attach(2000, 500, taskCan1Test2, "taskCan1Test2");
    stime.scheduler.attach(2000, 1000, taskCan2Test, "taskCan2Test");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
