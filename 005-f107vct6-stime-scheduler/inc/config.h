#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

// ============================================================================
// ----------------------------------------------------------------------------
// one-third-core includes
#include "core-utils.h"

// ----------------------------------------------------------------------------
#define _STIME_USE_SYSTICK
#define _STIME_2K_TICK
#define _STIME_USE_SCHEDULER
#include "core-stime.h"

// ----------------------------------------------------------------------------
#define _CONSOLE_USE_UART5_PC12PD2
#include "core-console.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
