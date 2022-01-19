#include "can.h"
#include <string.h>

// ============================================================================
#if defined(CAN_IS_USED)

// ============================================================================
#if defined(STM32F107xC)

// STM32F107 with 25 external crystal that the APB1 = 36M
#define CAN_BIT_RATE_NUM 15
// 36000000 / 4 / ( 1 + 6 + 2) = 1000K
static const uint32_t bit_rate_table[CAN_BIT_RATE_NUM][5] = {
    // bit rate, CAN_SJW,   CAN_BS1, CAN_BS2,    CAN_Prescaler
    { 15, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 150 },  // 15K
    { 20, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 200 },   // 20k
    { 25, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 90 },   // 25k
    { 40, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 100 },   // 40k
    { 50, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 45 },   // 50k
    { 62, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 36 },   // 62.5k
    { 80, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 50 },    // 80k
    { 100, CAN_SJW_1TQ, CAN_BS1_5TQ, CAN_BS2_2TQ, 45 },   // 100K
    { 125, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 18 },  // 125K
    { 200, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 20 },   // 200K
    { 250, CAN_SJW_1TQ, CAN_BS1_13TQ, CAN_BS2_2TQ, 9 },   // 250k
    { 400, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 10 },   // 400K
    { 500, CAN_SJW_1TQ, CAN_BS1_5TQ, CAN_BS2_2TQ, 9 },    // 500K
    { 800, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 5 },    // 800K
    { 1000, CAN_SJW_1TQ, CAN_BS1_6TQ, CAN_BS2_2TQ, 4 },   // 1000K
};
#endif  // STM32F107xC

// ----------------------------------------------------------------------------
#if defined(STM32F407xx)

// STM32F407 with 12M external crystal that the APB1 = 42M
// 42000000 / 3 / ( 1 + 9 + 4) = 1000K
#define CAN_BIT_RATE_NUM 14
static const uint32_t bit_rate_table[CAN_BIT_RATE_NUM][5] = {
    // bit rate, CAN_SJW, CAN_BS1, CAN_BS2, CAN_Prescaler
    // to be verified ----------
    { 15, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 200 },   // 15K
    { 20, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 100 },  // 20k
    { 25, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 120 },   // 25k
    { 40, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 50 },   // 40k
    { 50, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 60 },    // 50k
    { 62, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 48 },    // 62.5k
    { 80, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 25 },   // 80k
    { 100, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 20 },  // 100K
    { 125, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 24 },   // 125K
    { 200, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 10 },  // 200K
    { 250, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 12 },   // 250k
    { 400, CAN_SJW_1TQ, CAN_BS1_15TQ, CAN_BS2_5TQ, 5 },   // 400K
    // verified ----------
    { 500, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 6 },   // 500K
                                                         // 800K unavailable
    { 1000, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 3 },  // 1000K
};
#endif  // STM32F407xx

// ============================================================================
static void CAN_Bit_Rate_Process(uint16_t b_rate_k, CAN_InitTypeDef* can_init) {
    uint8_t iter;
    for (iter = 0; iter < CAN_BIT_RATE_NUM; iter++) {
        if (b_rate_k == bit_rate_table[iter][0]) {
            can_init->SyncJumpWidth = bit_rate_table[iter][1];
            can_init->TimeSeg1 = bit_rate_table[iter][2];
            can_init->TimeSeg2 = bit_rate_table[iter][3];
            can_init->Prescaler = bit_rate_table[iter][4];
            break;
        }
    }
    if (iter == CAN_BIT_RATE_NUM) {
        can_error(0, "CAN_Bit_Rate_Process(): wrong bit rate!\r\n");
    }
}

// ============================================================================
bool can_check_bit_rate(uint16_t b_rate_k) {
    for (uint8_t i = 0; i < CAN_BIT_RATE_NUM; i++) {
        if ((uint32_t)b_rate_k == bit_rate_table[i][0]) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// type: CAN_RTR_DATA, or CAN_RTR_REMOTE
HAL_StatusTypeDef can_send_packet(CAN_HandleTypeDef* handle, uint16_t can_id,
                                  uint32_t type, uint8_t* data, uint8_t len) {
    CAN_TxHeaderTypeDef tx_header;
    tx_header.IDE = CAN_ID_STD;
    tx_header.StdId = can_id;
    tx_header.ExtId = 0;
    tx_header.RTR = type;
    tx_header.DLC = len;
    tx_header.TransmitGlobalTime = DISABLE;
    uint8_t tx_data[8];
    for (int i = 0; i < len; i++) {
        tx_data[i] = data[i];
    }
    uint32_t mailbox;
    HAL_StatusTypeDef state =
        HAL_CAN_AddTxMessage(handle, &tx_header, tx_data, &mailbox);
    // is this the right way to check mailbox?
    if ((mailbox != CAN_TX_MAILBOX0) && (mailbox != CAN_TX_MAILBOX1)
        && (mailbox != CAN_TX_MAILBOX2)) {
        can_printk(0, "%s(): no mail box.", __func__);
        return HAL_ERROR;
    }
    return state;
}

// ============================================================================
void can_rx_print(const char* canx, CAN_RxHeaderTypeDef msg, uint8_t* data) {
    can_printk(0, YLW "%s IRQ " NOC "(id: 0x%04X, DLC: %d):", canx, msg.StdId,
               msg.DLC);
    for (int i = 0; i < msg.DLC; i++) {
        can_printk(0, " %02X", data[i]);
    }
    can_printk(0, "\r\n");
}

// ============================================================================
void can_irq_show_registration(const char* str, CanIrqNode_t* node,
                               uint8_t num) {
    CONSOLE_PRINTF_SEG;
    can_printk(0, "%s IRQ registration | %2d callback", str, num);
    if (num <= 1) {
        can_printk(0, "\r\n");
    }
    else {
        can_printk(0, "s\r\n");
    }
    for (uint8_t i = 0; i < num; i++) {
        can_printk(0, "COB ID = 0x%03X : %s\r\n", node[i].this_.cob_id,
                   node[i].this_.descr);
    }
    CONSOLE_PRINTF_SEG;
}

// ============================================================================
bool can_irq_attach(CanIrqNode_t* node, uint8_t num, uint16_t cob_id,
                    can_irq_hook hook, const char* str) {
    uint8_t len;
    size_t str_len = strlen(str);
    if (str_len >= _CAN_IRQ_DESCR_SIZE - 1) {
        len = (uint8_t)(_CAN_IRQ_DESCR_SIZE - 1);
    }
    else {
        len = (uint8_t)str_len;
    }
    // you cannot attach two callback functions to one ID
    if (num > 0) {
        for (uint8_t i = 0; i < num; i++) {
            if (node[0].this_.cob_id == cob_id) {
                return false;
            }
        }
    }
    if (num == 0) {
        node[0].this_.cob_id = cob_id;
        bzero(node[0].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(node[0].this_.descr, str, len);
        node[0].this_.descr[len] = '\0';
        node[0].this_.hook = hook;
        node[0].next_ = NULL;
    }
    else {
        node[num].this_.cob_id = cob_id;
        bzero(node[num].this_.descr, _CAN_IRQ_DESCR_SIZE);
        strncpy(node[num].this_.descr, str, len);
        node[num].this_.descr[len] = '\0';
        node[num].this_.hook = hook;
        node[num - 1].next_ = &node[num];
    }
    return true;
}

// ============================================================================
// arguments
// bit_rate (Kbps):
// 15, 20, 25, 40, 50, 62, 80, 100, 125, 200, 250, 400, 500, 800, 1000
// mode:
//     CAN_MODE_NORMAL
//     CAN_MODE_LOOPBACK
//     CAN_MODE_SILENT
//     CAN_MODE_SILENT_LOOPBACK
void can_settings(CAN_HandleTypeDef* hcan, uint16_t b_rate_k, uint32_t mode) {
    hcan->Init.Mode = mode;
    CAN_Bit_Rate_Process(b_rate_k, &(hcan->Init));
    hcan->Init.TimeTriggeredMode = DISABLE;
    hcan->Init.AutoBusOff = DISABLE;
    hcan->Init.AutoWakeUp = DISABLE;
    hcan->Init.AutoRetransmission = DISABLE;
    hcan->Init.ReceiveFifoLocked = DISABLE;
    hcan->Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(hcan) != HAL_OK) {
        can_error(0, "failed to setup the CAN interface.\r\n");
    }

    // filter: to accept all, may have some bug because mixed all CANs
    CAN_FilterTypeDef can_filter = { 0 };
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
    if (HAL_CAN_ConfigFilter(hcan, &can_filter) != HAL_OK) {
        can_error(0, "failed to setup CAN filter.\r\n");
    }
}

// ============================================================================
#endif  // CAN_IS_USED
