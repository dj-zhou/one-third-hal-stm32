#pragma once

// ============================================================================
#include "config.h"

#include "config-can.h"
#include "uart-console.h"

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

#ifdef CONSOLE_IS_USED
#define can_printf(...) (console.printf(__VA_ARGS__))
#define can_printk(...) (console.printk(__VA_ARGS__))
#define can_error(...) (console.error(__VA_ARGS__))
#else
#define can_printf(...) ({ ; })
#define can_printk(...) ({ ; })
#define can_error(...) ({ ; })
#endif

// FreeRTOS related configuration
// clang-format off
#ifdef RTOS_USE_FREERTOS
    #ifndef _CAN_PREEMPTION_PRIORITY
        #define _CAN_PREEMPTION_PRIORITY     6   // cannot be equal or smaller to 5
    #endif
    #ifndef _CAN_SUB_PRIORITY
        #define _CAN_SUB_PRIORITY            0
    #endif
// ----------------------------------------------------------------------------
// bare-metal
#else
    #ifndef _CAN_PREEMPTION_PRIORITY
        #define _CAN_PREEMPTION_PRIORITY     1
    #endif
    #ifndef _CAN_SUB_PRIORITY
        #define _CAN_SUB_PRIORITY            0
    #endif
#endif
// clang-format on

// ----------------------------------------------------------------------------
// clang-format off
#ifndef _CAN_IRQ_MAX_NUM
    #define _CAN_IRQ_MAX_NUM     20
#endif

#ifndef _CAN_IRQ_DESCR_SIZE
    #define _CAN_IRQ_DESCR_SIZE  30
#endif

typedef void (*can_irq_hook)(CAN_RxHeaderTypeDef*, uint8_t*);

// clang-format off
typedef struct CanIrqCpnt_s {
    uint16_t     cob_id;
    char         descr[_CAN_IRQ_DESCR_SIZE];
    can_irq_hook hook;
} CanIrqCpnt_t;

typedef struct CanIrqNode_s {
    struct CanIrqCpnt_s  this_;
    struct CanIrqNode_s* next_;
} CanIrqNode_t;
// clang-format on

// ----------------------------------------------------------------------------
// functions to be called inside the CAN module
bool can_check_bit_rate(uint16_t b_rate_k);
HAL_StatusTypeDef can_send_packet(CAN_HandleTypeDef* handle, uint16_t can_id,
                                  uint32_t type, uint8_t* data, uint8_t len);
void can_rx_print(const char* canx, CAN_RxHeaderTypeDef msg, uint8_t* data);
void can_irq_show_registration(const char* str, CanIrqNode_t* node,
                               uint8_t num);
bool can_irq_attach(CanIrqNode_t* node, uint8_t num, uint16_t cob_id,
                    can_irq_hook hook, const char* str);
void can_settings(CAN_HandleTypeDef* hcan, uint16_t b_rate_k, uint32_t mode);

typedef struct {
    void (*attach)(uint16_t, can_irq_hook, const char*);
    void (*show)(void);
} CanIrq_t;

typedef struct {
    CAN_HandleTypeDef hcan;
    void (*config)(uint16_t b_rate_k, uint32_t mode);
    HAL_StatusTypeDef (*sendData)(uint16_t can_id, uint8_t* data, uint8_t len);
    HAL_StatusTypeDef (*sendRemote)(uint16_t can_id, uint8_t* data,
                                    uint8_t len);
    bool (*checkBitRate)(uint16_t b_rate_k);
    CanIrq_t irq;
    // place holders
    void (*filter)(void);

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
