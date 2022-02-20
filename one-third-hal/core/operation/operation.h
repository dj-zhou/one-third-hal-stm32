#pragma once

#include "config.h"

#include "general.h"
#include <stdint.h>

// ============================================================================
// The operation module is designed to operate on the things that is not an
// object. For exampele, unlike usart1, usart2, etc, we can define RingBuffer
// for different ports, however, we do not have ringbuffer1, ringbuffer2 to be
// defined in the library just like the uart ports. Instead, we declare an array
// of uint8_t data and config it to be a ringbuffer, then we use this opreation
// module to work on it.

// ============================================================================
WARN_UNUSED_RESULT RingBuffer_t RingBufferInit(uint8_t* buffer, uint16_t size);
void RingBufferReset(RingBuffer_t* rb);
bool RingBufferPush(RingBuffer_t* rb, uint8_t data);
bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len);
bool RingBufferPop(RingBuffer_t* rb, uint8_t* ret);
bool RingBufferPopN(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
bool RingBufferTail(RingBuffer_t* rb, uint16_t pos);
bool RingBufferAdded(RingBuffer_t* rb, uint16_t count);
void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width);
void RingBufferError(RingBufferError_e e);
WARN_UNUSED_RESULT int8_t RingBufferSearch(RingBuffer_t* rb,
                                           const RingBufferInfo_t ring_info);
void RingBufferInsight(RingBuffer_t* rb);
WARN_UNUSED_RESULT int8_t RingBufferFetch(RingBuffer_t* rb, uint8_t* array,
                                          uint16_t size);

void ringbuffer_set_header(uint8_t* header, uint8_t len,
                           RingBufferInfo_t* ring_info);
void ringbuffer_set_length(uint8_t pos, uint8_t width,
                           RingBufferInfo_t* ring_info);
uint16_t ringbuffer_get_length(uint8_t* data, RingBufferInfo_t* ring_info);
void ringbuffer_set_type(uint8_t pos, uint8_t width,
                         RingBufferInfo_t* ring_info);
uint16_t ringbuffer_get_type(uint8_t* data, RingBufferInfo_t* ring_info);
void ringbuffer_set_device(RinBufferSearchDevice_e dev,
                           RingBufferInfo_t* ring_info);

// ----------------------------------------------------------------------------
typedef struct {
    WARN_UNUSED_RESULT RingBuffer_t (*init)(uint8_t* buffer, uint16_t size);
    void (*reset)(RingBuffer_t* rb);
    bool (*push)(RingBuffer_t* rb, uint8_t data);
    bool (*pushN)(RingBuffer_t* rb, uint8_t* data, uint16_t len);
    bool (*pop)(RingBuffer_t* rb, uint8_t* ret);
    bool (*popN)(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
    bool (*tail)(RingBuffer_t* rb, uint16_t pos);
    bool (*added)(RingBuffer_t* rb, uint16_t count);
    void (*show)(RingBuffer_t* rb, char style, uint16_t width);
    void (*error)(RingBufferError_e e);
    WARN_UNUSED_RESULT int8_t (*search)(RingBuffer_t* rb, RingBufferInfo_t);
    void (*insight)(RingBuffer_t* rb);
    WARN_UNUSED_RESULT int8_t (*fetch)(RingBuffer_t* rb, uint8_t* array,
                                       uint16_t size);
} OpRingBufferApi_t;

// ============================================================================
// clang-format off
typedef struct {
    OpRingBufferApi_t ringbuffer;
} OperationApt_t;

extern OperationApt_t op;
// clang-format on
