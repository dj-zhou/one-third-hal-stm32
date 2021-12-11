#include "can.h"
#include <string.h>

// ============================================================================
#if defined(CAN_IS_USED)

#if defined(STM32F407xx)

// STM32F407 with 12M external crystal that the APB1 = 42M
// 42000000 / 3 / ( 1 + 11 + 2) = 1000K
#define CAN_BIT_RATE_NUM 14
static const uint32_t CAN_bit_rate_table[CAN_BIT_RATE_NUM][5] = {
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
    { 500, CAN_SJW_1TQ, CAN_BS1_9TQ, CAN_BS2_4TQ, 6 },    // 500K
                                                          // 800K unavailable
    // generated from CubeMX
    { 1000, CAN_SJW_1TQ, CAN_BS1_11TQ, CAN_BS2_2TQ, 3 },  // 1000K
};
#endif  // STM32F407xx

// =============================================================================================
static void CAN_Bit_Rate_Process(uint16_t b_rate_k, CAN_InitTypeDef* can_init) {
    uint8_t iter;
    for (iter = 0; iter < CAN_BIT_RATE_NUM; iter++) {
        if (b_rate_k == CAN_bit_rate_table[iter][0]) {
            can_init->SyncJumpWidth = ( uint8_t )CAN_bit_rate_table[iter][1];
            can_init->TimeSeg1 = ( uint8_t )CAN_bit_rate_table[iter][2];
            can_init->TimeSeg2 = ( uint8_t )CAN_bit_rate_table[iter][3];
            can_init->Prescaler = ( uint8_t )CAN_bit_rate_table[iter][4];
            break;
        }
    }
    if (iter == CAN_BIT_RATE_NUM) {
        console.error("CAN_Bit_Rate_Process(): wrong bit rate!\r\n");
    }
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
void InitCanSettings(CAN_HandleTypeDef* hcan, uint16_t b_rate_k,
                     uint32_t mode) {
    hcan->Init.Mode = ( uint32_t )mode;
    CAN_Bit_Rate_Process(b_rate_k, &(hcan->Init));
    hcan->Init.TimeTriggeredMode = DISABLE;
    hcan->Init.AutoBusOff = DISABLE;
    hcan->Init.AutoWakeUp = DISABLE;
    hcan->Init.AutoRetransmission = DISABLE;
    hcan->Init.ReceiveFifoLocked = DISABLE;
    hcan->Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(hcan) != HAL_OK) {
        console.error("failed to setup the CAN interface\r\n");
    }
}
// ============================================================================
#endif  // CAN_IS_USED
