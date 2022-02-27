#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

// ============================================================================
// one-third-core includes
// ----------------------------------------------------------------------------
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_1K_TICK
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_USART2_PA2PA3
#include "uart-console.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
