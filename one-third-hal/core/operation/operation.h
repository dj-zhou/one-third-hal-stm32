#pragma once

#include "ring-buffer.h"

// ============================================================================
// The operation module is designed to operate on the things that is not an
// object. For exampele, unlike usart1, usart2, etc, we can define RingBuffer
// for different ports, however, we do not have ringbuffer1, ringbuffer2 to be
// defined in the library just like the uart ports. Instead, we declare an array
// of uint8_t data and config it to be a ringbuffer, then we use this opreation
// module to work on it.

// ============================================================================
// clang-format off
typedef struct {
    RingBufferOpApi_t ringbuffer;
} OperationApt_t;

extern OperationApt_t op;
// clang-format on
