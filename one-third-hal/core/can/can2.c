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

    InitCanSettings(&(can2.hcan), b_rate_k, mode);
    // start CAN
    if (HAL_CAN_Start(&(can2.hcan)) != HAL_OK) {
        console.error("failed to start can\r\n");
    };
    // enable interrupts
    HAL_CAN_ActivateNotification(&(can2.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);

    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
}

// ----------------------------------------------------------------------------
// clang-format off
CanApi_t can2 = {
    .config      = InitCan2             ,
};
// clang-format on

// ============================================================================
#endif  // CAN2_EXISTS && CAN2_IS_USED
