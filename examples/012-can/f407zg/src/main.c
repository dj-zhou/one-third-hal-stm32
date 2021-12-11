#include "config.h"
#include <math.h>

// =============================================================================
static void taskCan1Test(void) {
    CAN_TxHeaderTypeDef can_tx_header;
    can_tx_header.IDE = CAN_ID_STD;
    can_tx_header.StdId = 0x011;
    can_tx_header.RTR = CAN_RTR_DATA;
    can_tx_header.DLC = 2;
    can_tx_header.TransmitGlobalTime = DISABLE;

    uint8_t data[8];
    data[0] = 0xAA;
    data[1] = 0xBB;

    uint32_t mailbox;

    static uint32_t loop_count = 0;
    if (HAL_CAN_AddTxMessage(&(can1.hcan), &can_tx_header, data, &mailbox)
        != HAL_OK) {
        console.printf("%5d: failed to send a CAN1 message\r\n", loop_count);
    }
    else {
        console.printf("%5d: sent a CAN1 message\r\n", loop_count);
    }
    loop_count++;
}

// =============================================================================
static void taskCan2Test(void) {
    CAN_TxHeaderTypeDef can_tx_header;
    can_tx_header.IDE = CAN_ID_STD;
    can_tx_header.StdId = 0x022;
    can_tx_header.RTR = CAN_RTR_DATA;
    can_tx_header.DLC = 2;
    can_tx_header.TransmitGlobalTime = DISABLE;

    uint8_t data[8];
    data[0] = 0x88;
    data[1] = 0x99;

    uint32_t mailbox;

    static uint32_t loop_count = 0;
    if (HAL_CAN_AddTxMessage(&(can2.hcan), &can_tx_header, data, &mailbox)
        != HAL_OK) {
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
    can1.config(500, CAN_MODE_NORMAL);
    can2.config(500, CAN_MODE_NORMAL);
    // tasks -----------
    stime.scheduler.attach(2000, 1, taskCan1Test, "taskCan1Test");
    stime.scheduler.attach(2000, 1000, taskCan2Test, "taskCan2Test");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
