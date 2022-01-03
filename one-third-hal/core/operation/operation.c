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
        // visualization
        .show = RingBufferShow,
        // search and see result
        .header  = RingBufferHeader ,
        .search  = RingBufferSearch ,
        .insight = RingBufferInsight,
        // attach a process function (defined outside)
        .attach  = RingbufferAttach,
    },
};
// clang-format on
