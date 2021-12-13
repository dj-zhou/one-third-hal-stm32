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

    CAN_FilterTypeDef can_filter;
    can_filter.FilterBank = 0;
    can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
    can_filter.FilterIdHigh = 0x0000;
    can_filter.FilterIdLow = 0x0000;
    can_filter.FilterMaskIdHigh = 0x0000;
    can_filter.FilterMaskIdLow = 0x0000;
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    can_filter.FilterActivation = ENABLE;
    can_filter.SlaveStartFilterBank = 0;
    if (HAL_CAN_ConfigFilter(&(can1.hcan), &can_filter) != HAL_OK) {
        console.error("filter setup wrong.\r\n");
    }
    // start CAN
    if (HAL_CAN_Start(&(can1.hcan)) != HAL_OK) {
        console.error("failed to start CAN1\r\n");
    };
    // enable interrupts
    HAL_CAN_ActivateNotification(&(can1.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, _CAN_PREEMPTION_PRIORITY,
                         _CAN_SUB_PRIORITY);
    __HAL_CAN_ENABLE_IT(&(can1.hcan), CAN1_RX0_IRQn);
}

// ----------------------------------------------------------------------------
void CAN1_RX0_IRQHandler(void) {
    CAN_RxHeaderTypeDef msg;
    uint8_t data[8];
    HAL_CAN_GetRxMessage(&(can1.hcan), CAN_RX_FIFO0, &msg, data);
    console.printf("I received a packet\r\n");
    HAL_CAN_IRQHandler(&(can1.hcan));
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
