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
// ringbuffer

// clang-format off
#if !defined(_RINGBUFFER_HEADER_MAX_LEN)
    #define _RINGBUFFER_HEADER_MAX_LEN     5
#endif
#if !defined(_RINGBUFFER_PACKETS_MAX_FOUND)
    #define _RINGBUFFER_PACKETS_MAX_FOUND     5
#endif
// clang-format on

// ----------------------------------------------------------------------------
#if defined(CONSOLE_IS_USED)
#define ringbuffer_printf(...) (console.printf(__VA_ARGS__))
#define ringbuffer_printk(...) (console.printk(__VA_ARGS__))
#define ringbuffer_error(...) (console.error(__VA_ARGS__))
#else
#define ringbuffer_printf(...) ({ ; })
#define ringbuffer_printk(...) ({ ; })
#define ringbuffer_error(...) ({ ; })
#endif

// ----------------------------------------------------------------------------
// ringbuffer description
//        head : index of first item (oldest data)
//        tail : index of the next position of the last item (latest data)
//    capacity : the total volume of the ringbuffer
//       count : the actual amount of valid data (uint8_t size)

// when initialized, head = -1, tail = 0, count = 0.
// when last byte popped out, head = -1, tail = 0, count = 0.

// if head == tail:
// 1. the ringbuffer is full and the head is where it is pointed

// normally, the buffer and head and tail are like this:
//    xxxx    data    data    data    data    data    data    xxxx    xxxx
//            head                                            tail

// ----------------------------------------------------------------------------
// clang-format off
typedef enum RingBufferError {
    RINGBUFFER_NO_ERROR =  0,
    RINGBUFFER_ERR_OOR  = -1,  // OOR = out of range
    RINGBUFFER_ERR_CNM  = -2,  // CNM = can not move
    RINGBUFFER_ERR_SNCP = -3,  // SNCP = search no complete packet
    RINGBUFFER_ERR_PNI  = -4,  // PNI = pop negative number of items
    RINGBUFFER_ERR_PTMI = -5,  // PTMI = pop too many items
} RingBufferError_e;

typedef enum RingBufferInitState {
    RINGBUFFER_INITIALIZED = 1,
    RINGBUFFER_RESETTED    = 2,
    RINGBUFFER_EMPTIED     = 3,
} RingBufferInitState_e;

#pragma pack(1)
typedef struct {
    int16_t  head;
    uint16_t tail;
    uint16_t capacity;
    uint16_t count;
    RingBufferInitState_e  state;
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
    bool searched;
    uint16_t pos[_RINGBUFFER_PACKETS_MAX_FOUND];  // index of the header
    uint16_t dist[_RINGBUFFER_PACKETS_MAX_FOUND]; // distance to next header
    int8_t   count;
} RingBufferIndex_t;
#pragma pack()

#pragma pack(1)
typedef struct RingBuffer_s {
    uint8_t*           data;
    RingBufferState_t  state;
    RingBufferHeader_t header;
    RingBufferIndex_t  index;
} RingBuffer_t;
#pragma pack()

// ============================================================================
WARN_UNUSED_RESULT RingBuffer_t RingBufferInit(uint8_t* buffer, uint16_t size);
void RingBufferReset(RingBuffer_t* rb);
bool RingBufferPush(RingBuffer_t* rb, uint8_t data);
bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len);
bool RingBufferPop(RingBuffer_t* rb, uint8_t* ret);
bool RingBufferPopN(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width);
void RingBufferHeader(RingBuffer_t* rb, uint8_t* array, uint8_t size);
WARN_UNUSED_RESULT int8_t RingBufferSearch(RingBuffer_t* rb);
void RingBufferInsight(RingBuffer_t* rb);
WARN_UNUSED_RESULT int8_t RingBufferFetch(RingBuffer_t* rb, uint8_t* array,
                                          uint16_t size);

// clang-format on
// ----------------------------------------------------------------------------
typedef struct {
    WARN_UNUSED_RESULT RingBuffer_t (*init)(uint8_t* buffer, uint16_t size);
    void (*reset)(RingBuffer_t* rb);
    bool (*push)(RingBuffer_t* rb, uint8_t data);
    bool (*pushN)(RingBuffer_t* rb, uint8_t* data, uint16_t len);
    bool (*pop)(RingBuffer_t* rb, uint8_t* ret);
    bool (*popN)(RingBuffer_t* rb, uint8_t* ret, uint16_t len);
    void (*show)(RingBuffer_t* rb, char style, uint16_t width);
    void (*header)(RingBuffer_t* rb, uint8_t array[], uint8_t size);
    WARN_UNUSED_RESULT int8_t (*search)(RingBuffer_t* rb);
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