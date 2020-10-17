#ifndef __CORE_UTILS_H
#define __CORE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// clang-format off
typedef struct {
    HAL_StatusTypeDef ( *systemClockInit )( void );
} CoreUtilsApi_t;

extern CoreUtilsApi_t utils;
// clang-format on

#ifdef __cplusplus
}
#endif

#endif  // __CORE_UTILS_H

// ============================================================================
// supported system clock
// MCU-type     HSE_VALUE   SYSCLK    PCLK1     PCLK2     ADCCLK??
// STM32F107xC  25000000    72000000  36000000  72000000  xx
