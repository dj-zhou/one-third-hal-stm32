#include "can.h"
#include <string.h>

// ============================================================================
#if defined(CAN2_EXISTS) && defined(CAN2_IS_USED)

// ----------------------------------------------------------------------------
// TO verify: is this the right way to initialize?
static CanIrqNode_t can2_node[_CAN_IRQ_MAX_NUM] = { 0 };
static uint8_t can2_node_num = 0;

static void IrqAttachCan2(uint16_t cob_id, can_irq_hook hook, const char* str) {
    uint8_t len;
    uint8_t str_len = strlen(str);
    if (str_len >= _CAN_IRQ_DESCR_SIZE - 1) {
        len = _CAN_IRQ_DESCR_SIZE - 1;
    }
    else {
        len = str_len;
    }

    if (can2_node_num == 0) {
        can2_node[0].this_.cob_id = cob_id;
        bzero(can2_node[0].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(can2_node[0].this_.descr, str, len);
        can2_node[0].this_.descr[len] = '\0';
        can2_node[0].this_.hook = hook;
        can2_node[0].next_ = NULL;
    }
    else {
        can2_node[can2_node_num].this_.cob_id = cob_id;
        bzero(can2_node[can2_node_num].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(can2_node[can2_node_num].this_.descr, str, len);
        can2_node[can2_node_num].this_.descr[len] = '\0';
        can2_node[can2_node_num].this_.hook = hook;
        can2_node[can2_node_num - 1].next_ = &can2_node[can2_node_num];
    }
    can2_node_num++;
}

// ============================================================================
static void IrqShowCan2(void) {
    console.printf("CAN2 registered IRQ functions are:\r\n");
    for (uint8_t i = 0; i < can2_node_num; i++) {
        console.printf("COB ID = 0x%03X : %s\r\n", can2_node[i].this_.cob_id,
                       can2_node[i].this_.descr);
    }
}

// ============================================================================
#if defined(STM32F407xx)
void InitCan2_PB13PB12(void) {
    GPIO_InitTypeDef gpio = { 0 };
    utils.clock.enableGpio(GPIOB);
    // CAN2 GPIO Configuration
    // PB12     ------> CAN2_RX
    // PB13     ------> CAN2_TX
    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &gpio);
}
#endif
// ----------------------------------------------------------------------------
static void InitCan2(uint16_t b_rate_k, uint32_t mode) {
    can2.hcan.Instance = CAN2;
#if defined(_USE_CAN2_PB6PB5)
    InitCan2_PB6PB5();  // todo
#endif
#if defined(_USE_CAN2_PB13PB12)
    InitCan2_PB13PB12();
#endif
    utils.clock.enableCan(can2.hcan.Instance);

    can_settings(&(can2.hcan), b_rate_k, mode);

    // enable interrupts
    // also use FIFO0 ?? seems right, to verify
    HAL_CAN_ActivateNotification(&(can2.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, _CAN_PREEMPTION_PRIORITY,
                         _CAN_SUB_PRIORITY);

    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
    // the following does not work! do not use and do not delete
    // __HAL_CAN_ENABLE_IT(&(can2.hcan), CAN2_RX0_IRQn);

    // start CAN
    if (HAL_CAN_Start(&(can2.hcan)) != HAL_OK) {
        console.error("failed to start CAN2\r\n");
    };
}

// ----------------------------------------------------------------------------
void __attribute__((weak)) CAN2_RX0_IRQHandler(void) {
    CAN_RxHeaderTypeDef msg = { 0 };
    uint8_t data[8] = { 0 };
    HAL_CAN_GetRxMessage(&(can2.hcan), CAN_RX_FIFO0, &msg, data);

    for (uint8_t i = 0; i < can2_node_num; i++) {
        if (can2_node[i].this_.cob_id == msg.StdId) {
            can2_node[i].this_.hook(&msg, data);
            return;
        }
    }
#if defined(_CAN2_IRQ_SHOW_UNKNOWN_MSG)
    can_rx_print("CAN2", msg, data);
#endif
}

// ----------------------------------------------------------------------------
static bool CheckBitRateCan2(uint16_t b_rate_k) {
    return can_check_bit_rate(b_rate_k);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef SendDataCan2(uint16_t can_id, uint8_t* data,
                                      uint8_t len) {
    return can_send_packet(&(can2.hcan), can_id, CAN_RTR_DATA, data, len);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef SendRemoteCan2(uint16_t can_id, uint8_t* data,
                                        uint8_t len) {
    return can_send_packet(&(can2.hcan), can_id, CAN_RTR_REMOTE, data, len);
}

// ----------------------------------------------------------------------------
// clang-format off
CanApi_t can2 = {
    .config       = InitCan2        ,
    .sendData     = SendDataCan2    ,
    .sendRemote   = SendRemoteCan2  ,
    .checkBitRate = CheckBitRateCan2,
    .irq.attach   = IrqAttachCan2   ,
    .irq.show     = IrqShowCan2     ,
};
// clang-format on

// ============================================================================
#endif  // CAN2_EXISTS && CAN2_IS_USED
