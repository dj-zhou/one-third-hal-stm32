#include "operation.h"

// ============================================================================
// clang-format off
OperationApt_t op = {
    .ringbuffer = {
        // initialization
        .init  = RingBufferInit ,
        .reset = RingBufferReset,
        // push and pop
        .push  = RingBufferPush ,
        .pushN = RingBufferPushN,
        .pop   = RingBufferPop  ,
        .popN  = RingBufferPopN ,
        .tail  = RingBufferTail ,
        .added = RingBufferAdded,
        // visualization
        .show = RingBufferShow  ,
        .error = RingBufferError,
        // search and see result
        .search  = RingBufferSearch ,
        .insight = RingBufferInsight,
        .fetch   = RingBufferFetch  ,
    },
};
// clang-format on
