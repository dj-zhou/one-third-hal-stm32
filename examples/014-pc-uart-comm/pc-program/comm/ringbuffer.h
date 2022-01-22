#pragma once

#include <stdint.h>

typedef enum RingBufferError {
    RINGBUFFER_NO_ERROR = 0,
    RINGBUFFER_HEADER_TOO_SMALL = -1,
    RINGBUFFER_HEADER_TOO_LARGE = -2,
    RINGBUFFER_JUST_INITIALIZED = -3,
    RINGBUFFER_FEW_COUNT = -4,
    RINGBUFFER_FIND_NO_HEADER = -5,
    RINGBUFFER_FETCH_DES_SMALL = -6,
} RingBufferError_e;

typedef enum RingBufferInitState {
    RINGBUFFER_INITIALIZED = 1,
    RINGBUFFER_RESETTED = 2,
    RINGBUFFER_EMPTIED = 3,
} RingBufferInitState_e;

#pragma pack(1)
typedef struct {
    int16_t head;
    uint16_t tail;
    uint16_t capacity;
    uint16_t count;
    RingBufferInitState_e state;
} RingBufferState_t;
#pragma pack()

#pragma pack(1)
typedef struct {
    bool searched;
    uint16_t* pos;   // index of the header
    uint16_t* dist;  // distance to next header
    uint8_t size;
    int8_t count;
} RingBufferIndex_t;
#pragma pack()

class RingBuffer {
public:
    RingBuffer();
    RingBuffer(uint16_t size, uint8_t max_header_found);
    ~RingBuffer();

    void init(uint16_t size, uint8_t max_header_found);
    void reset();
    bool push(uint8_t data);
    bool push(uint8_t* data, uint16_t len);
    bool pop(uint8_t* ret);
    bool pop(uint8_t* ret, uint16_t len);
    bool tail(uint16_t pos);
    bool added(uint16_t count);
    void show(char style, uint16_t width);
    void error(RingBufferError_e e);
    int8_t search(uint8_t* header, uint8_t header_size);
    void insight();
    int8_t fetch(uint8_t* array, uint16_t size);

private:
    uint16_t index(uint16_t idx);
    uint8_t* buffer_;
    RingBufferState_t state_;
    RingBufferIndex_t index_;
};

// WARN_UNUSED_RESULT