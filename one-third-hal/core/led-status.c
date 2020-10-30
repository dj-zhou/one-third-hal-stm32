#include "led-status.h"
#include "general-utils.h"
#include "stime-scheduler.h"
#include "uart-console.h"

// ============================================================================
static LedHeartBeat_e mode_;
static uint16_t       tick_toggle_;
static uint16_t       tick_period_;

// ============================================================================
static void CalculateParameters( LedHeartBeat_e mode ) {
    switch ( mode ) {
    case LED_OFF:
    case LED_ON:
        tick_toggle_ = 0;  // not used
        break;
    case LED_BLINK1:
        tick_toggle_ = 50;
        tick_period_ = 100;
        break;
    case LED_BLINK2:
        tick_toggle_ = 25;
        tick_period_ = 50;
        break;
    case LED_BLINK3:
        tick_toggle_ = 16;
        tick_period_ = 33;  // not precise to 3Hz
        break;
    case LED_BLINK4:
        tick_toggle_ = 12;
        tick_period_ = 25;
        break;
    case LED_PWM_MODE:;  // todo
        break;
    case LED_DOUBLE_BLINK:
        tick_toggle_ = 15;
        tick_period_ = 150;
        break;
    default:
        tick_toggle_ = mode / 10;
        tick_period_ = 100;
        break;
    }
    if ( _LED_HEARTBEAT_BLINK == 10 ) {
        ;  // do nothing
    }
    else if ( _LED_HEARTBEAT_BLINK == 5 ) {
        tick_toggle_ *= 2;
        tick_period_ *= 2;
    }
    else {
        console.printf( "%s(): _LED_HEARTBEAT_BLINK cannot be %d\r\n",
                        _LED_HEARTBEAT_BLINK );
    }
}

// ============================================================================
static void LedToggleHeartBeat( void ) {
    utils.pin.toggle( _LED_HEARTBEAT_PORT, _LED_HEARTBEAT_PIN );
}

// ============================================================================
static void LedHeartBeatOn( bool v ) {
#if defined( _LED_LOW_DRIVE )
    HAL_GPIO_WritePin( _LED_HEARTBEAT_PORT, 1 << _LED_HEARTBEAT_PIN, ~v );
    return;
#endif
    HAL_GPIO_WritePin( _LED_HEARTBEAT_PORT, 1 << _LED_HEARTBEAT_PIN, v );
}

// ============================================================================
static void LedToggleError( void ) {
    utils.pin.toggle( _LED_ERROR_PORT, _LED_ERROR_PIN );
}

// ============================================================================
static void LedErrorOn( bool v ) {
#if defined( _LED_LOW_DRIVE )
    HAL_GPIO_WritePin( _LED_ERROR_PORT, 1 << _LED_ERROR_PIN, ~v );
    return;
#endif
    HAL_GPIO_WritePin( _LED_ERROR_PORT, 1 << _LED_ERROR_PIN, v );
}

// ============================================================================
// this function must be called every 10ms to make the time/frequency consistent
#if defined( _STIME_USE_SCHEDULER )
static void blinkHeartBeat( void ) {
    static uint16_t loop = 0;
    switch ( mode_ ) {
    case LED_OFF:
        LedHeartBeatOn( false );
        break;
    case LED_ON:
        LedHeartBeatOn( true );
        break;
    case LED_PWM_MODE:
        break;
    case LED_DOUBLE_BLINK:
        if ( loop < tick_toggle_ ) {
            LedHeartBeatOn( true );
        }
        else if ( ( tick_toggle_ <= loop ) && ( loop < tick_toggle_ * 2 ) ) {
            LedHeartBeatOn( false );
        }
        else if ( ( tick_toggle_ * 2 <= loop )
                  && ( loop < tick_toggle_ * 3 ) ) {
            LedHeartBeatOn( true );
        }
        else {
            LedHeartBeatOn( false );
        }
        loop++;
        if ( loop > tick_period_ ) {
            loop = 0;
        }
        break;
    default:
        if ( loop < tick_toggle_ ) {
            LedHeartBeatOn( true );
        }
        else {
            LedHeartBeatOn( false );
        }
        loop++;
        if ( loop > tick_period_ ) {
            loop = 0;
        }
        break;
    }
}
#endif

// ============================================================================
static void LedGpioConfig( LedHeartBeat_e mode ) {
    mode_ = mode;
    CalculateParameters( mode );
    // GPIO initialization ---------
    utils.pin.mode( _LED_HEARTBEAT_PORT, _LED_HEARTBEAT_PIN,
                    GPIO_MODE_OUTPUT_PP );
    utils.pin.mode( _LED_ERROR_PORT, _LED_ERROR_PIN, GPIO_MODE_OUTPUT_PP );

    LedHeartBeatOn( false );
    LedErrorOn( false );

#if defined( _STIME_USE_SCHEDULER )
    // blink heartbeat task registration ---------
    // heart beat task, call it for every 10 ms
    stime.scheduler.regist( _LED_HEARTBEAT_BLINK, 5, blinkHeartBeat,
                            "blinkHeartBeat" );
#endif
}

// ============================================================================
// clang-format off
LedStatusApi_t led = {
    .config          = LedGpioConfig      ,
    .toggleHeartBeat = LedToggleHeartBeat , // should be removed 
    .toggleError     = LedToggleError     , // should be removed 
};
// clang-format on
