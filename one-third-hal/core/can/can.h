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

#if defined(CAN1_EXISTS) || defined(CAN2_EXISTS)
    #define CAN_IS_USED
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(CAN_IS_USED)
typedef void (*CAN_IRQ_Hook)(CAN_RxHeaderTypeDef*);

// functions to be called inside the CAN module
void InitCanSettings(CAN_HandleTypeDef* hcan, uint16_t b_rate_k, uint32_t mode);

typedef struct {
    CAN_HandleTypeDef hcan;
    void (*config)(uint16_t b_rate_k, uint32_t mode);
    void (*filter)(void);  // a place holder
    uint8_t (*send)(CAN_TxHeaderTypeDef* msg, uint8_t* data);
    void (*irqRegister)(uint16_t, CAN_IRQ_Hook, const char*);
    void (*irqRegisterUpdate)(uint8_t, uint8_t);
    void (*irqRegisterShow)(void);
    bool (*checkBitRate)(uint16_t bit_rate);
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
