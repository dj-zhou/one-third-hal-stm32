#include "id.h"

#if defined(_USE_ID)
uint8_t id_;

// ============================================================================
static void IdConfig(void) {
    id_ = 0;
}

// ============================================================================
static uint8_t IdGet(void) {
    return id_;
}

// ============================================================================
static void IdSet(uint8_t id) {
    id_ = id;
}

// ============================================================================
// clang-format off
SystemId_t sid = {
    .config = IdConfig,
    .get    = IdGet   ,
    .set    = IdSet   ,
};
// clang-format on

#endif  // _USE_ID
