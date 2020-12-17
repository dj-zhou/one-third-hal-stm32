#ifndef __GENERAL_H
#define __GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

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
    #define _CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#endif

#if !defined(_SET_BIT)
    #define _SET_BIT(var, pos) ((var) |= (1 << (pos)))
#endif

#if !defined(_RESET_BIT)
    #define _RESET_BIT(var, pos) ((var) &= (~(1 << (pos))))
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
// clang-format on

// ============================================================================
// to print with color (escaping)
#define NOC "\033[0m"
#define GRY "\033[0;30m"
#define RED "\033[0;31m"
#define GRN "\033[0;32m"
#define YLW "\033[0;33m"
#define BLU "\033[0;34m"
#define PRP "\033[0;35m"
#define CYN "\033[0;36m"
#define WHT "\033[0;37m"
#define HGRY "\033[1;30m"
#define HRED "\033[1;31m"
#define HGRN "\033[1;32m"
#define HYLW "\033[1;33m"
#define HBLU "\033[1;34m"
#define HPRP "\033[1;35m"
#define HCYN "\033[1;36m"
#define HWHT "\033[1;37m"

// ============================================================================
// general used things
// clang-format off
#if defined(STM32F030x8) || defined(STM32F103xB) || defined(STM32F107xC) \
    || defined(STM32F303xE)
    #define    GPIO_13RD_SPEED_HIGH    GPIO_SPEED_FREQ_HIGH
#elif defined(STM32F407xx) || defined(STM32F427xx) || defined(STM32F746xx) \
    || defined(STM32F767xx)
    #define    GPIO_13RD_SPEED_HIGH    GPIO_SPEED_FREQ_VERY_HIGH
#else
#error to implement and verify!
#endif
// clang-format on

// used in ring-buffer ---------------------
#pragma pack(1)
typedef struct RingBuffer_s {
    uint8_t* buffer;
    int16_t  head;
    int16_t  tail;
    uint16_t capacity;
    uint16_t count;
    bool     initialized;
} RingBuffer_t;
#pragma pack()

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __GENERAL_H
