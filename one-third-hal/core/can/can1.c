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

    InitCanSettings(&(can1.hcan), b_rate_k, mode);
    // start CAN
    if (HAL_CAN_Start(&(can1.hcan)) != HAL_OK) {
        console.error("failed to start can\r\n");
    };
    // enable interrupts
    HAL_CAN_ActivateNotification(&(can1.hcan), CAN_IT_RX_FIFO0_MSG_PENDING);
    // CAN1 interrupt Init

    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

// ----------------------------------------------------------------------------
// clang-format off
CanApi_t can1 = {
    .config = InitCan1,
};
// clang-format on

// ============================================================================
#endif  // CAN1_EXISTS && CAN1_IS_USED
