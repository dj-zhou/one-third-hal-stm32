#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

// clang-format off
#if !defined(_RINGBUFFER_HEADER_MAX_LEN)
    #define _RINGBUFFER_HEADER_MAX_LEN     5
#endif
#if !defined(_RINGBUFFER_PACKETS_MAX_FOUND)
    #define _RINGBUFFER_PACKETS_MAX_FOUND     5
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

// if head == tail:
// 1. the ringbuffer is full and the head is where it is pointed

// normally, the buffer and head and tail are like this:
//    xxxx    data    data    data    data    data    data    xxxx    xxxx
//            head                                            tail

// clang-format off
typedef enum RingBufferError {
    RINGBUFFER_NO_ERROR =  0,
    RINGBUFFER_ERR_OOR  = -1,  // OOR = out of range
    RINGBUFFER_ERR_CNM  = -2,  // CNM = can not move
    RINGBUFFER_ERR_SNCP = -3,  // SNCP = search no complete packet
    RINGBUFFER_ERR_PNI  = -4,  // PNI = pop negative number of items
    RINGBUFFER_ERR_PTMI = -5,  // PTMI = pop too many items
} RingBufferError_e;



#pragma pack(1)
typedef struct {
    int16_t  head;
    int16_t  tail;
    uint16_t capacity;
    uint16_t count;
    uint8_t  is_initialized;
} RingBufferState_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint8_t data[_RINGBUFFER_HEADER_MAX_LEN];
    uint8_t size;
} RingBufferHeader_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    uint16_t pos[_RINGBUFFER_PACKETS_MAX_FOUND];    // should be changed
    uint16_t dist[_RINGBUFFER_PACKETS_MAX_FOUND];  // how many bytes to the
                                                    // next header
    uint8_t  count;
} RingBufferIndex_t;
#pragma pack()

typedef void (*ringbuffer_hook)(void*);
#pragma pack(1)
typedef struct RingBuffer_s {
    uint8_t*           data;
    RingBufferState_t  state;
    RingBufferHeader_t header;
    RingBufferIndex_t  index;
    ringbuffer_hook    process;
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
void RingBufferHeader(RingBuffer_t* rb, uint8_t* array, uint8_t size);
RingBufferError_e RingBufferSearch(RingBuffer_t* rb);
RingBufferError_e RingBufferMoveHead(RingBuffer_t* rb, int16_t pos);
void RingBufferInsight(RingBuffer_t* rb);

// ============================================================================
typedef struct {
    WARN_UNUSED_RESULT RingBuffer_t (*init)(uint8_t* buffer, uint16_t size);
    void (*reset)(RingBuffer_t* rb);
    bool (*push)(RingBuffer_t* rb, uint8_t data);
    bool (*pushN)(RingBuffer_t* rb, uint8_t* data, uint16_t len);
    bool (*pop)(RingBuffer_t* rb, uint8_t* ret);
    bool (*popN)(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
    void (*show)(RingBuffer_t* rb, char style, uint16_t width);
    void (*header)(RingBuffer_t* rb, uint8_t array[], uint8_t size);
    // search the ringbuffer for some pattern, and record the locations to index
    WARN_UNUSED_RESULT RingBufferError_e (*search)(RingBuffer_t* rb);
    void (*insight)(RingBuffer_t* rb);
    RingBufferError_e (*move)(RingBuffer_t* rb, int16_t pos);
} RingBufferOpApi_t;

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __RING_BUFFER_H
