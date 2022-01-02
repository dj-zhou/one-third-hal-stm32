#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

// clang-format off
#if !defined(_RINGBUFFER_MAX_PATTERN_FOUND)
    #define _RINGBUFFER_MAX_PATTERN_FOUND     5
#endif
// clang-format on

// ============================================================================
// ringbuffer description
//        head : index of first item (oldest data)
//        tail : index of the next position of the last item (latest data)
//    capacity : the total volume of the ringbuffer
//       count : the actual amount of valid data (uint8_t size)

// when initialized, head == -1, tail = 0, count = 0. This is the only
// situation that head == -1

// if head == tail,  the ringbuffer is full and the head is where it is pointed

// normally, the buffer and head and tail are like this:
//    xxxx    data    data    data    data    data    data    xxxx    xxxx
//            head                                            tail

// clang-format off
typedef enum RingBufferError {
    RINGBUFFER_NO_ERROR =  0,
    RINGBUFFER_ERR_OOR  = -1,  // OOR = out of range
    RINGBUFFER_ERR_CNM  = -2,  // CNM = can not move
    RINGBUFFER_ERR_SPS  = -3,  // SPS = search pattern small
    RINGBUFFER_ERR_SNCP = -4,  // SNCP = search no complete packet
    RINGBUFFER_ERR_PNI  = -5,  // PNI = pop negative number of items
    RINGBUFFER_ERR_PTMI = -6,  // PTMI = pop too many items
} RingBufferError_e;
#pragma pack(1)
typedef struct {
    uint16_t pos[_RINGBUFFER_MAX_PATTERN_FOUND];    // should be changed
    uint16_t count[_RINGBUFFER_MAX_PATTERN_FOUND];  // how many bytes to the
                                                    // next pattern
    uint8_t  found;
} RingBufferIndex_t;
#pragma pack()

#pragma pack(1)
typedef struct RingBuffer_s {
    uint8_t* data;
    int16_t  head;
    int16_t  tail;
    uint16_t capacity;
    uint16_t count;
    uint8_t  is_initialized;
} RingBuffer_t;
#pragma pack()
// clang-format on

// ============================================================================
WARN_UNUSED_RESULT RingBuffer_t RingBufferInit(uint8_t* buffer, uint16_t size);
void RingBufferReset(RingBuffer_t* rb);
bool RingBufferPush(RingBuffer_t* rb, uint8_t data);
bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len);
bool RingBufferPop(RingBuffer_t* rb, uint8_t* ret);
bool RingBufferPopN(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width);
RingBufferError_e RingBufferSearch(RingBuffer_t* rb, uint8_t* pattern,
                                   uint8_t len, RingBufferIndex_t* index);

RingBufferError_e RingBufferMoveHead(RingBuffer_t* rb, int16_t pos);
void RingBufferInsight(RingBufferIndex_t* index);

// ============================================================================
typedef struct {
    WARN_UNUSED_RESULT RingBuffer_t (*init)(uint8_t* buffer, uint16_t size);
    void (*reset)(RingBuffer_t* rb);
    bool (*push)(RingBuffer_t* rb, uint8_t data);
    bool (*pushN)(RingBuffer_t* rb, uint8_t* data, uint16_t len);
    bool (*pop)(RingBuffer_t* rb, uint8_t* ret);
    bool (*popN)(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
    void (*show)(RingBuffer_t* rb, char style, uint16_t width);

    // search the ringbuffer for some pattern, and record the locations to index
    RingBufferError_e (*search)(RingBuffer_t* rb, uint8_t* pattern, uint8_t len,
                                RingBufferIndex_t* index);
    RingBufferError_e (*move)(RingBuffer_t* rb, int16_t pos);
    void (*insight)(RingBufferIndex_t* index);
} RingBufferOpApi_t;

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __RING_BUFFER_H
