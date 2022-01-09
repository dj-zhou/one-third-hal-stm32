#ifndef __GENERAL_H
#define __GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// clang-format off
// 0x1234 --> 0x3412
#if !defined(_SWAP_16)
    #define _SWAP_16(x) (x = (uint16_t)((x >> 8) | (x << 8)))
#endif

// 0x 12345678 --> 0x78563412
#if !defined(_SWAP_32)
    #define _SWAP_32(x)                                                     \
        (x = ((x >> 24) | ((x >> 8) & 0x0000ff00) | ((x << 8) & 0x00ff0000) \
            | (x << 24)))
#endif

#if !defined(_CHECK_BIT)
    #define _CHECK_BIT(var, pos) (!!((var) & (1 << (pos))))
#endif

#if !defined(set_bit_8)
    #define set_bit_8(var, pos) ((var) = (uint8_t)((var) | (1 << (pos))))
#endif
#if !defined(set_bit_16)
    #define set_bit_16(var, pos) ((var) = (uint16_t)((var) | (1 << (pos))))
#endif
#if !defined(set_bit_32)
    #define set_bit_32(var, pos) ((var) = (uint32_t)((var) | (1 << (pos))))
#endif

#if !defined(reset_bit_8)
    #define reset_bit_8(var, pos) ((var) = (uint8_t)((var) & (~(1 << (pos)))))
#endif
#if !defined(reset_bit_16)
    #define reset_bit_16(var, pos) ((var) = (uint16_t)((var) & (~(1 << (pos)))))
#endif
#if !defined(reset_bit_32)
    #define reset_bit_32(var, pos) ((var) = (uint32_t)((var) & (~(1 << (pos)))))
#endif

#if !defined(sizeof_array)
    #define sizeof_array(x) (sizeof(x) / sizeof((x)[0]))
#endif

#if !defined(offsetof)
    #define offsetof(TYPE, MEMBER) ((size_t) & (( TYPE* )0)->MEMBER)
#endif

#if !defined(container_of)
    #define container_of(ptr, type, member)                      \
        ({                                                       \
            const typeof((( type* )0)->member)* __mptr = (ptr);  \
            ( type* )(( char* )__mptr - offsetof(type, member)); \
        })
#endif

#if !defined(WARN_UNUSED_RESULT)
    #define WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#endif
// clang-format on

// ============================================================================
// to print with color (escaping)
// clang-format off
#define NOC  "\033[0m"
#define GRY  "\033[0;30m"
#define RED  "\033[0;31m"
#define GRN  "\033[0;32m"
#define YLW  "\033[0;33m"
#define BLU  "\033[0;34m"
#define PRP  "\033[0;35m"
#define CYN  "\033[0;36m"
#define WHT  "\033[0;37m"
#define HGRY "\033[1;30m"
#define HRED "\033[1;31m"
#define HGRN "\033[1;32m"
#define HYLW "\033[1;33m"
#define HBLU "\033[1;34m"
#define HPRP "\033[1;35m"
#define HCYN "\033[1;36m"
#define HWHT "\033[1;37m"
// clang-format on

// ============================================================================
// general used things
// clang-format off
#if defined(STM32F030x8) || defined(STM32F103xB) || defined(STM32F107xC) \
    || defined(STM32F303xE)
    #define    GPIO_13RD_SPEED_HIGH    GPIO_SPEED_FREQ_HIGH
#elif defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx) || defined(STM32H750xx)
    #define    GPIO_13RD_SPEED_HIGH    GPIO_SPEED_FREQ_VERY_HIGH
#else
#error to implement and verify!
#endif
// clang-format on

// ============================================================================
// ringbuffer
// ringbuffer can be used in many other modules, so we do not have this
// interface, however, just keep it here, do not delete
// clang-format off
// #if !defined(_RINGBUFFER_HEADER_MAX_LEN)
//     #define _RINGBUFFER_HEADER_MAX_LEN     5
// #endif
// #if !defined(_RINGBUFFER_PACKETS_MAX_FOUND)
//     #define _RINGBUFFER_PACKETS_MAX_FOUND     5
// #endif
// clang-format on

#define RINGBUFFER_HEADER_MAX_LEN 5
#define RINGBUFFER_PACKETS_MAX_FOUND 5

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
    uint8_t data[RINGBUFFER_HEADER_MAX_LEN];
    uint8_t size;
} RingBufferHeader_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    bool searched;
    uint16_t pos[RINGBUFFER_PACKETS_MAX_FOUND];  // index of the header
    uint16_t dist[RINGBUFFER_PACKETS_MAX_FOUND]; // distance to next header
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
#ifdef __cplusplus
}
#endif

#endif  // __GENERAL_H
