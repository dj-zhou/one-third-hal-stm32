#include "led-status.h"
#include "general-utils.h"
#include "stime-scheduler.h"
#include "uart-console.h"
#include <math.h>

// ============================================================================
static LedHeartBeat_e    heartbeat_mode_;
static uint16_t          tick_toggle_;
static uint16_t          tick_period_;
static TIM_TypeDef*      PWM_TIMx_    = NULL;
static uint8_t           pwm_channel_ = 255;
static uint8_t           pwm_alter_;
static float             pwm_step_size_;
static TIM_HandleTypeDef htim_;

// ============================================================================
static void CalculateParameters(LedHeartBeat_e mode) {
    switch (mode) {
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
    case LED_DOUBLE_BLINK:
        tick_toggle_ = 15;
        tick_period_ = 150;
        break;
    default:
        tick_toggle_ = mode / 10;
        tick_period_ = 100;
        break;
    }
    if (_LED_HEARTBEAT_TASK_MS == 10) {
        pwm_step_size_ = 0.03;
    }
    else if (_LED_HEARTBEAT_TASK_MS == 5) {
        tick_toggle_ *= 2;
        tick_period_ *= 2;
        pwm_step_size_ = 0.015;
    }
    else {
        console.error("%s(): _LED_HEARTBEAT_TASK_MS cannot be %d\r\n",
                      _LED_HEARTBEAT_TASK_MS);
    }
}

// ============================================================================
static void LedToggleHeartBeat(void) {
    utils.pin.toggle(_LED_HEARTBEAT_PORT, _LED_HEARTBEAT_PIN);
}

// ============================================================================
static void LedHeartBeatOn(bool v) {
#if defined(_LED_LOW_DRIVE)
    HAL_GPIO_WritePin(_LED_HEARTBEAT_PORT, 1 << _LED_HEARTBEAT_PIN, !v);
    return;
#endif
    HAL_GPIO_WritePin(_LED_HEARTBEAT_PORT, 1 << _LED_HEARTBEAT_PIN, v);
}

// ============================================================================
static void LedToggleError(void) {
    utils.pin.toggle(_LED_ERROR_PORT, _LED_ERROR_PIN);
}

// ============================================================================
static void LedErrorOn(bool v) {
#if defined(_LED_LOW_DRIVE)
    HAL_GPIO_WritePin(_LED_ERROR_PORT, 1 << _LED_ERROR_PIN, !v);
    return;
#endif
    HAL_GPIO_WritePin(_LED_ERROR_PORT, 1 << _LED_ERROR_PIN, v);
}

// ============================================================================
// this function must be called every 10ms to make the time/frequency consistent
#if defined(_STIME_USE_SCHEDULER)
static void blinkHeartBeat(void) {
    static uint16_t loop = 0;
    // in PWM mode ---------------------------
    if (heartbeat_mode_ == LED_PWM_MODE) {
        static float x = 0;
        x += pwm_step_size_;
        if (x > 3.1415926 * 2) {
            x = 0;
        }
        uint16_t pwm_value = (uint16_t)((sin(x) + 1) / 2 * 2000);
        __HAL_TIM_SET_COMPARE(&htim_, pwm_channel_, pwm_value);
        return;
    }
    // not in PWM mode ---------------------------
    switch (heartbeat_mode_) {
    case LED_OFF:
        LedHeartBeatOn(false);
        break;
    case LED_ON:
        LedHeartBeatOn(true);
        break;
    case LED_PWM_MODE:
        break;
    case LED_DOUBLE_BLINK:
        if (loop < tick_toggle_) {
            LedHeartBeatOn(true);
        }
        else if ((tick_toggle_ <= loop) && (loop < tick_toggle_ * 2)) {
            LedHeartBeatOn(false);
        }
        else if ((tick_toggle_ * 2 <= loop) && (loop < tick_toggle_ * 3)) {
            LedHeartBeatOn(true);
        }
        else {
            LedHeartBeatOn(false);
        }
        loop++;
        if (loop > tick_period_) {
            loop = 0;
        }
        break;
    default:
        if (loop < tick_toggle_) {
            LedHeartBeatOn(true);
        }
        else {
            LedHeartBeatOn(false);
        }
        loop++;
        if (loop > tick_period_) {
            loop = 0;
        }
        break;
    }
}
#endif

// ============================================================================
// stm32f767: TIM4_CH2: PB7, TIM3_CH3: PB0, TIM12_CH1: PB14
// stm32f427: TIM3_CH4: PB1, TIM2_CH2: PB3, TIM2_CH4: PB11
static void LedHeartBeatVerifyPwm(void) {
#if defined(GPIOB_EXISTS)
    if (_LED_HEARTBEAT_PORT == GPIOB) {
        // PB0 -------- known boards: f767zi
        if (_LED_HEARTBEAT_PIN == 0) {  // TIM3_CH3
            g_config_timer_used |= 1 << 3;
#if defined(TIM3_EXISTS)
            PWM_TIMx_    = TIM3;
            pwm_channel_ = TIM_CHANNEL_3;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF2_TIM3;  // verified
#endif
            return;
        }
        // PB1 -------- known boards: FireDragon
        if (_LED_HEARTBEAT_PIN == 1) {  // TIM3_CH4
            g_config_timer_used |= 1 << 3;
#if defined(TIM3_EXISTS)
            PWM_TIMx_    = TIM3;
            pwm_channel_ = TIM_CHANNEL_4;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF2_TIM3;  // verified
#endif
            return;
        }
        // PB3 -------- known boards: FireDragon
        if (_LED_HEARTBEAT_PIN == 3) {  // TIM2_CH2
            g_config_timer_used |= 1 << 2;
#if defined(TIM2_EXISTS)
            PWM_TIMx_    = TIM2;
            pwm_channel_ = TIM_CHANNEL_2;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF1_TIM2;  // verified
#endif
            return;
        }
        // PB7 -------- known boards: f767zi
        if (_LED_HEARTBEAT_PIN == 7) {  // TIM4_CH2
            g_config_timer_used |= 1 << 4;
#if defined(TIM4_EXISTS)
            PWM_TIMx_    = TIM4;
            pwm_channel_ = TIM_CHANNEL_2;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF2_TIM4;  // verified
#endif
            return;
        }
        // PB11 -------- known boards: FireDragon
        if (_LED_HEARTBEAT_PIN == 11) {  // TIM2_CH4
            g_config_timer_used |= 1 << 2;
#if defined(TIM2_EXISTS)
            PWM_TIMx_    = TIM2;
            pwm_channel_ = TIM_CHANNEL_4;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF1_TIM2;  // verified
#endif
            return;
        }
        // PB14 --------known boards: f767zi
        if (_LED_HEARTBEAT_PIN == 14) {  // TIM12_CH1
            g_config_timer_used |= 1 << 12;
#if defined(TIM12_EXISTS)
            PWM_TIMx_    = TIM12;
            pwm_channel_ = TIM_CHANNEL_1;
#endif
#if defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF9_TIM12;  // verified
#endif
            return;
        }
    }
#endif

#if defined(GPIOE_EXISTS)
    if (_LED_HEARTBEAT_PORT == GPIOE) {
        // PE11 --------known boards: f407zg
        if (_LED_HEARTBEAT_PIN == 11) {  // TIM1_CH2
            g_config_timer_used |= 1 << 1;
#if defined(TIM1_EXISTS)
            PWM_TIMx_    = TIM1;
            pwm_channel_ = TIM_CHANNEL_2;
#endif
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F767xx)
            pwm_alter_ = GPIO_AF1_TIM1;  // verified
#endif
            return;
        }
    }
#endif
    // if reach this line, the pin is not supported for PWM, or need to check
    // and update this library
    if ((PWM_TIMx_ == NULL) && (pwm_channel_ == 255)) {
        console.error("%s(): not a PWM pin, or you can refer to datasheet and "
                      "extend this library.\r\n",
                      __func__);
    }
}

// ============================================================================
static void LedHeartBeatPwmConfig(void) {
#if defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    utils.pin.alter(_LED_HEARTBEAT_PORT, _LED_HEARTBEAT_PIN, pwm_alter_);
    utils.clock.enableTimer(PWM_TIMx_);

    // timer base initialization -------------------
    htim_.Instance               = PWM_TIMx_;
    htim_.Init.Prescaler         = 100 - 1;
    htim_.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim_.Init.Period            = 2000;
    htim_.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim_.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    while (HAL_TIM_OC_Init(&htim_) != HAL_OK) {
        // Error_Handler();
    }
    // timer master initialization?? --------------
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    while (HAL_TIMEx_MasterConfigSynchronization(&htim_, &sMasterConfig)
           != HAL_OK) {
        // Error_Handler();
    }

    // OC initialization -------------------
    TIM_OC_InitTypeDef sConfigOC = { 0 };

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    while (HAL_TIM_OC_ConfigChannel(&htim_, &sConfigOC, pwm_channel_)
           != HAL_OK) {
        // Error_Handler();
    }
    HAL_TIM_OC_Start(&htim_, pwm_channel_);
    __HAL_TIM_SET_COMPARE(&htim_, pwm_channel_, 0);
#endif
}

// ============================================================================
// should add error mode as an argument as well
static void LedGpioConfig(LedHeartBeat_e heatbeat_mode) {
    ( void )LedToggleError;      // to avoid compile error
    ( void )LedToggleHeartBeat;  // to avoid compile error
    ( void )htim_;               // to avoid compile error
    ( void )pwm_alter_;          // to avoid compile error
    heartbeat_mode_ = heatbeat_mode;
    // some assert
    if ((_LED_HEARTBEAT_PIN > 15) || (_LED_ERROR_PIN > 15)) {
        console.error(
            "%s(): _LED_HEARTBEAT_PIN or _LED_ERROR_PIN out of range\r\n",
            __func__);
    }
    CalculateParameters(heatbeat_mode);
    if (heatbeat_mode != LED_PWM_MODE) {
        utils.pin.mode(_LED_HEARTBEAT_PORT, _LED_HEARTBEAT_PIN,
                       GPIO_MODE_OUTPUT_PP);
        utils.pin.mode(_LED_ERROR_PORT, _LED_ERROR_PIN, GPIO_MODE_OUTPUT_PP);
        LedHeartBeatOn(false);
        LedErrorOn(false);
    }
    else {                        // heatbeat_mode == LED_PWM_MODE
        LedHeartBeatVerifyPwm();  // if pass, means the PWM pin is good
        LedHeartBeatPwmConfig();
    }

#if defined(_STIME_USE_SCHEDULER)
    // blink heartbeat task registration ---------
    // heart beat task, call it for every 10 ms or 5 ms
    stime.scheduler.attach(_LED_HEARTBEAT_TASK_MS, 2, blinkHeartBeat,
                           "blinkHeartBeat");
#endif
}

// ============================================================================
// clang-format off
LedStatusApi_t led = {
    .config          = LedGpioConfig     ,
    // .toggleHeartBeat = LedToggleHeartBeat, // should be removed
    // .toggleError     = LedToggleError    , // should be removed
};
// clang-format on
