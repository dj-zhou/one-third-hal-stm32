#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

// ============================================================================
// one-third-core includes
// ----------------------------------------------------------------------------
#include "general-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_1K_TICK
#define _STIME_USE_SCHEDULER
#include "stime-scheduler.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_UART5_PC12PD2
#include "uart-console.h"

// ============================================================================
#ifdef __cplusplus
}
#endif
