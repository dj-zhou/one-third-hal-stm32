#pragma once

#include <mutex>
#include <stdint.h>

enum class RingBufferError {
    NO_ERROR = 0,
    HEADER_TOO_SMALL = -1,
    HEADER_TOO_LARGE = -2,
    JUST_INITIALIZED = -3,
    FEW_COUNT = -4,
    FIND_NO_HEADER = -5,
    FETCH_DES_SMALL = -6,
    LEN_POS_ERROR = -7,
    LEN_WIDTH_ERROR = -8,
};

enum class RingBufferInitState {
    INITIALIZED = 1,
    RESETTED = 2,
};
#pragma pack(1)
typedef struct {
    int16_t head;
    uint16_t tail;
    uint16_t capacity;
    uint16_t count;
    RingBufferInitState state;
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
    void show(char style, uint16_t width);
    void error(int e);
    int8_t search(uint8_t* header, uint8_t header_size, uint8_t len_pos,
                  uint8_t len_width);
    void insight();
    int8_t fetch(uint8_t* array, uint16_t size);

private:
    int16_t index(int16_t idx);
    uint16_t getPacketLen(uint16_t head_pos, uint8_t len_pos,
                          uint8_t len_width);
    uint8_t* buffer_;
    RingBufferState_t state_;
    RingBufferIndex_t index_;
    std::mutex mutex_;
};

// WARN_UNUSED_RESULT
