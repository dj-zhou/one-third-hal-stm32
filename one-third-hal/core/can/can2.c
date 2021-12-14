#include "can.h"

// ============================================================================
#if defined(CAN2_EXISTS) && defined(CAN2_IS_USED)

#if defined(STM32F407xx)
void InitCan1_PB13PB12() {
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
    InitCan1_PB6PB5();  // todo
#endif
#if defined(_USE_CAN2_PB13PB12)
    InitCan1_PB13PB12();
#endif
    utils.clock.enableCan(can2.hcan.Instance);

    can_settings(&(can2.hcan), b_rate_k, mode);

    // start CAN
    if (HAL_CAN_Start(&(can2.hcan)) != HAL_OK) {
        console.error("failed to start CAN2\r\n");
    };

    // enable interrupts
    HAL_CAN_ActivateNotification(&(can2.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, _CAN_PREEMPTION_PRIORITY,
                         _CAN_SUB_PRIORITY);
    __HAL_CAN_ENABLE_IT(&(can2.hcan), CAN1_RX0_IRQn);
}

// ----------------------------------------------------------------------------
void CAN2_RX0_IRQHandler(void) {
    CAN_RxHeaderTypeDef msg;
    uint8_t data[8];
    HAL_CAN_GetRxMessage(&(can2.hcan), CAN_RX_FIFO0, &msg, data);
    console.printf("I received a packet\r\n");
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
};
// clang-format on

// ============================================================================
#endif  // CAN2_EXISTS && CAN2_IS_USED
