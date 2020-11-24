#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
#if !defined( _PWM_TIMER_COUNT )
#define _PWM_TIMER_COUNT ( 3 )
#endif
// ============================================================================
#include "config.h"

// this API of this PWM module only deal with timer and channel, not a specific
// GPIO group and its pin number

// PWM mode 1: OC mode

// Pulse mode 1: xxx

#pragma pack( 1 )
typedef struct {
    uint8_t  timer;  // low 4 bits: timer No., high 4 bits: mode
    uint32_t frequency;
    uint16_t pre_scale;  // save the data of pwm/pulse mode for future use
    uint16_t period;     // save the data of pwm/pulse mode for future use
    uint8_t GPIOx[2];  // GPIOx[0] low 4 bits: channel 1, high 4 bits: channel 2
                       // GPIOx[1] low 4 bits: channel 3, high 4 bits: channel 4
    uint8_t pin[2];    // pin[0] low 4 bits: channel 1, high 4 bits: channel 2
                       // pin[1] low 4 bits: channel 3, high 4 bits: channel 4
} PwmTimerStatus_t;
#pragma pack()

PwmTimerStatus_t pwm_timer_status_[_PWM_TIMER_COUNT];

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __PWM_H
