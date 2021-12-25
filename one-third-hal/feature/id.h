#pragma once
#include "config.h"

#include <stdint.h>

#if defined(_USE_ID)

// TODO: if can-node is used, then enable ot use ID
// ============================================================================

typedef struct {
    void (*config)(void);
    uint8_t (*get)(void);
    void (*set)(uint8_t);
} SystemId_t;
extern SystemId_t sid;

// ============================================================================
#endif  // _USE_ID
