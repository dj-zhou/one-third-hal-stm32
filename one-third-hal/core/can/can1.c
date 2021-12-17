#include "can.h"
#include <string.h>

// ============================================================================
#if defined(CAN1_EXISTS) && defined(CAN1_IS_USED)

// ----------------------------------------------------------------------------
// TO verify: is this the right way to initialize?
static CanIrqNode_t can1_node[_CAN_IRQ_MAX_NUM] = { 0 };
static uint8_t can1_node_num = 0;

static void IrqAttachCan1(uint16_t cob_id, can_irq_hook hook, const char* str) {
    uint8_t len;
    uint8_t str_len = strlen(str);
    if (str_len >= _CAN_IRQ_DESCR_SIZE - 1) {
        len = _CAN_IRQ_DESCR_SIZE - 1;
    }
    else {
        len = str_len;
    }

    if (can1_node_num == 0) {
        can1_node[0].this_.cob_id = cob_id;
        bzero(can1_node[0].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(can1_node[0].this_.descr, str, len);
        can1_node[0].this_.descr[len] = '\0';
        can1_node[0].this_.hook = hook;
        can1_node[0].next_ = NULL;
    }
    else {
        can1_node[can1_node_num].this_.cob_id = cob_id;
        bzero(can1_node[can1_node_num].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(can1_node[can1_node_num].this_.descr, str, len);
        can1_node[can1_node_num].this_.descr[len] = '\0';
        can1_node[can1_node_num].this_.hook = hook;
        can1_node[can1_node_num - 1].next_ = &can1_node[can1_node_num];
    }
    can1_node_num++;
}

// ============================================================================
static void IrqShowCan1(void) {
    console.printf("CAN1 registered IRQ functions are:\r\n");
    for (uint8_t i = 0; i < can1_node_num; i++) {
        console.printf("COB ID = 0x%03X : %s\r\n", can1_node[i].this_.cob_id,
                       can1_node[i].this_.descr);
    }
}

// ============================================================================
#if defined(STM32F107xC)
void InitCan1_PD1PD0() {
    utils.clock.enableGpio(GPIOD);
    // CAN1 GPIO Configuration
    // PD0     ------> CAN1_RX
    // PD1     ------> CAN1_TX
    GPIO_InitTypeDef gpio = { 0 };
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Pin = GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &gpio);

    __HAL_AFIO_REMAP_CAN1_3();
}
#endif

// ----------------------------------------------------------------------------
#if defined(STM32F407xx)
void InitCan1_PD1PD0() {
    utils.clock.enableGpio(GPIOD);
    GPIO_InitTypeDef gpio = { 0 };
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
void __attribute__((weak)) CAN1_RX0_IRQHandler(void) {
    CAN_RxHeaderTypeDef msg = { 0 };
    uint8_t data[8] = { 0 };
    HAL_CAN_GetRxMessage(&(can1.hcan), CAN_RX_FIFO0, &msg, data);

    for (uint8_t i = 0; i < can1_node_num; i++) {
        if (can1_node[i].this_.cob_id == msg.StdId) {
            can1_node[i].this_.hook(&msg, data);
            return;
        }
    }
#if defined(_CAN1_IRQ_SHOW_UNKNOWN_MSG)
    can_rx_print("CAN1", msg, data);
#endif
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
    .config       = InitCan1        ,
    .sendData     = SendDataCan1    ,
    .sendRemote   = SendRemoteCan1  ,
    .checkBitRate = CheckBitRateCan1,
    .irq.attach   = IrqAttachCan1   ,
    .irq.show     = IrqShowCan1     ,
};
// clang-format on

// ============================================================================
#endif  // CAN1_EXISTS && CAN1_IS_USED
