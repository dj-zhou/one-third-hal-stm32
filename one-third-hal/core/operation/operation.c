#include "operation.h"

// ============================================================================
// clang-format off
OperationApt_t op = {
    .ringbuffer = {
        .init   = RingBufferInit   ,
        .reset  = RingBufferReset  ,
        .push   = RingBufferPush   ,
        .pushN  = RingBufferPushN  ,
        .pop    = RingBufferPop    ,
        .popN   = RingBufferPopN   ,
        .show   = RingBufferShow   ,
        .header = RingBufferHeader ,
    },
};
// clang-format on
