#ifndef __LED_STATUS_H
#define __LED_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include <stdbool.h>

// ============================================================================
// clang-format off
#if !defined( _LED_HEARTBEAT_PORT )
    #define    _LED_HEARTBEAT_PORT      GPIOD
#endif

#if !defined( _LED_HEARTBEAT_PIN )
    #define     _LED_HEARTBEAT_PIN      ( 3 )
#endif

#if !defined( _LED_ERROR_PORT )
    #define    _LED_ERROR_PORT          GPIOD
#endif

#if !defined( _LED_ERROR_PIN )
    #define    _LED_ERROR_PIN           ( 4 )
#endif

#if !defined( _LED_HIGH_DRIVE ) || !defined( _LED_LOW_DRIVE )
    #define    _LED_HIGH_DRIVE
#endif

#if !defined( _LED_HEARTBEAT_BLINK )
    #define    _LED_HEARTBEAT_BLINK     ( 10 )
#endif
// clang-format on

// ============================================================================
// heartBeat mode
//   1. off/on
//   2. Blink continously at some freqeuncy, like one/two/three/four Blink per
//      second
//   3. turn on from 0.01 ~ 0.20 seocnd, and then turn off (1-0.01) ~ (1-0.80)
//      second
//   5. PWM mode, (breathing light mode), only special pins have this mode
typedef enum {
    LED_OFF          = 0,   // off
    LED_ON           = 1,   // on
    LED_BLINK1       = 2,   // Blink once per second
    LED_BLINK2       = 3,   // Blink twice per second
    LED_BLINK3       = 4,   // Blink three times per second
    LED_BLINK4       = 5,   // Blink four times per second
    LED_BEAT01       = 10,  // on for 10 ms, off for 990 ms
    LED_BEAT02       = 20,  // on for 20 ms, off for 980 ms
    LED_BEAT03       = 30,
    LED_BEAT04       = 40,
    LED_BEAT05       = 50,
    LED_BEAT06       = 60,
    LED_BEAT07       = 70,
    LED_BEAT08       = 80,
    LED_BEAT09       = 90,
    LED_BEAT10       = 100,
    LED_BEAT11       = 110,
    LED_BEAT12       = 120,
    LED_BEAT13       = 130,
    LED_BEAT14       = 140,
    LED_BEAT15       = 150,
    LED_BEAT16       = 160,
    LED_BEAT17       = 170,
    LED_BEAT18       = 180,
    LED_BEAT19       = 190,
    LED_BEAT20       = 200,  // on for 200 ms, off for 800 ms
    LED_PWM_MODE     = 201,  // breathing light mode, TODO
    LED_DOUBLE_BLINK = 202,  // flash twice in a period of 1.5 second
} LedHeartBeat_e;

// ============================================================================
// error mode
// some error modes use both heart beat led and error led for indicating
typedef enum {
    NoError            = 0,  // no error, led off
    Can1NoMailBoxError = 2,
    Can2NoMailBoxError = 3,
    E2promReadError    = 4,
} LedError_e;

// ============================================================================
// clang-format off
typedef struct {
    void ( *config )( LedHeartBeat_e mode );
    void ( *toggleHeartBeat )( void )      ; // should be removed 
    void ( *toggleError )( void )          ; // should be removed 
} LedStatusApi_t;
extern LedStatusApi_t led;
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __LED_STATUS_H
