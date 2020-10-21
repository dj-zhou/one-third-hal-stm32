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
#include "core-stime.h"

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  //__CONFIG_H
