#ifndef __CAN_H
#define __CAN_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
#include "config.h"

#include "config-can.h"

// clang-format off
#if defined(CAN1_EXISTS)
    #if defined(_USE_CAN1_PA12PA11) || defined(_USE_CAN1_PD1PD0) \
        || defined(_USE_CAN1_PB9PB8) || defined(_USE_CAN1_PH13PI9)
        #define CAN1_IS_USED
    #endif
#endif

#if defined(CAN2_EXISTS)
    #if defined(_USE_CAN2_PB6PB5) || defined(_USE_CAN2_PB13PB12)
        #define CAN2_IS_USED
    #endif
#endif

#if defined(CAN3_EXISTS) // todo
     // #define CAN3_IS_USED
#endif

#if defined(CAN1_IS_USED) || defined(CAN2_IS_USED) || defined(CAN3_IS_USED)
    #define CAN_IS_USED
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(CAN_IS_USED)

// functions to be called inside the CAN module
void can_settings(CAN_HandleTypeDef* hcan, uint16_t b_rate_k, uint32_t mode);
bool can_check_bit_rate(uint16_t b_rate_k);
HAL_StatusTypeDef can_send_packet(CAN_HandleTypeDef* handle, uint16_t can_id,
                                  uint32_t type, uint8_t* data, uint8_t len);

// todo
typedef void (*CAN_IRQ_Hook)(CAN_RxHeaderTypeDef*);

typedef struct {
    CAN_HandleTypeDef hcan;
    void (*config)(uint16_t b_rate_k, uint32_t mode);
    HAL_StatusTypeDef (*sendData)(uint16_t can_id, uint8_t* data, uint8_t len);
    HAL_StatusTypeDef (*sendRemote)(uint16_t can_id, uint8_t* data,
                                    uint8_t len);
    bool (*checkBitRate)(uint16_t b_rate_k);
    // place holders
    void (*filter)(void);
    void (*irqRegister)(uint16_t, CAN_IRQ_Hook, const char*);
    void (*irqRegisterUpdate)(uint8_t, uint8_t);
    void (*irqRegisterShow)(void);
    void (*rxPrint)(CAN_RxHeaderTypeDef* msg, uint8_t* data);
    void (*txPrint)(CAN_TxHeaderTypeDef* msg, uint8_t* data);
} CanApi_t;

#endif  // CAN_IS_USED
// ----------------------------------------------------------------------------
// clang-format off
#if defined(CAN1_EXISTS) && defined(CAN1_IS_USED)
    extern CanApi_t can1;
#endif

#if defined(CAN2_EXISTS) && defined(CAN2_IS_USED)
    extern CanApi_t can2;
#endif

#if defined(CAN3_EXISTS) && defined(CAN3_IS_USED)
    extern CanApi_t can3;
#endif
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __CAN_H
