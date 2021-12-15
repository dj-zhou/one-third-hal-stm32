#include "can.h"

// ============================================================================
#if defined(CAN1_EXISTS) && defined(CAN1_IS_USED)

#if defined(STM32F407xx)
void InitCan1_PD1PD0() {
    GPIO_InitTypeDef gpio = { 0 };
    utils.clock.enableGpio(GPIOD);
    // CAN1 GPIO Configuration
    // PD0     ------> CAN1_RX
    // PD1     ------> CAN1_TX
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOD, &gpio);
}
#endif

// ----------------------------------------------------------------------------
static void InitCan1(uint16_t b_rate_k, uint32_t mode) {
    can1.hcan.Instance = CAN1;
#if defined(_USE_CAN1_PA12PA11)
    InitCan1_PA12PA11();  // todo
#endif
#if defined(_USE_CAN1_PD1PD0)
    InitCan1_PD1PD0();
#endif
#if defined(_USE_CAN1_PB9PB8)
    InitCan1_PB9PB8();  // todo
#endif
#if defined(_USE_CAN1_PH13PI9)
    InitCan1_PH13PI9();  // todo
#endif
    utils.clock.enableCan(can1.hcan.Instance);

    can_settings(&(can1.hcan), b_rate_k, mode);

    // enable interrupts
    HAL_CAN_ActivateNotification(&(can1.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, _CAN_PREEMPTION_PRIORITY,
                         _CAN_SUB_PRIORITY);

    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    // the following does not work! do not use and do not delete
    // __HAL_CAN_ENABLE_IT(&(can1.hcan), CAN1_RX0_IRQn);

    // start CAN
    if (HAL_CAN_Start(&(can1.hcan)) != HAL_OK) {
        console.error("failed to start CAN1\r\n");
    };
}

// ----------------------------------------------------------------------------
void CAN1_RX0_IRQHandler(void) {
    CAN_RxHeaderTypeDef msg;
    uint8_t data[8];
    HAL_CAN_GetRxMessage(&(can1.hcan), CAN_RX_FIFO0, &msg, data);

    // if the CAN packet is not processed, then print it:
    console.printf("CAN1 receives: 0x%04X (%d): ", msg.StdId, msg.DLC);
    for (int i = 0; i < msg.DLC; i++) {
        console.printf(" %02X", data[i]);
    }
    console.printf("\r\n");
}

// ----------------------------------------------------------------------------
static bool CheckBitRateCan1(uint16_t b_rate_k) {
    return can_check_bit_rate(b_rate_k);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef SendDataCan1(uint16_t can_id, uint8_t* data,
                                      uint8_t len) {
    return can_send_packet(&(can1.hcan), can_id, CAN_RTR_DATA, data, len);
}

// ----------------------------------------------------------------------------
static HAL_StatusTypeDef SendRemoteCan1(uint16_t can_id, uint8_t* data,
                                        uint8_t len) {
    return can_send_packet(&(can1.hcan), can_id, CAN_RTR_REMOTE, data, len);
}

// ----------------------------------------------------------------------------
// clang-format off
CanApi_t can1 = {
    .config          = InitCan1           ,
    .sendData        = SendDataCan1       ,
    .sendRemote      = SendRemoteCan1     ,
    .checkBitRate    = CheckBitRateCan1   ,
};
// clang-format on

// ============================================================================
#endif  // CAN1_EXISTS && CAN1_IS_USED
