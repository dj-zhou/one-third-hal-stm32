#include "ringbuffer.h"

#include <iostream>
#include <magic_enum.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
int16_t RingBuffer::index(int16_t idx) {
    while (idx >= state_.capacity) {
        idx = (int16_t)(idx - state_.capacity);
    }
    // comparison is always false due to limited range of data type
    // [-Werror=type-limits], do not delete
    while (idx < 0) {
        idx = (int16_t)(idx + state_.capacity);
    }
    return idx;
}

// ============================================================================
RingBuffer::RingBuffer() {
    // state initialization
    state_.capacity = 0;
    state_.head = -1;
    state_.tail = 0;
    state_.count = 0;
    state_.state = RingBufferInitState::INITIALIZED;

    // search indices initialization
    index_.count = 0;
    index_.searched = false;
}

// ============================================================================
RingBuffer::RingBuffer(uint16_t size, uint8_t max_header_found) {
    init(size, max_header_found);
}

// ============================================================================
RingBuffer::~RingBuffer() {
    free(buffer_);
    free(index_.pos);
    free(index_.dist);
}

// ============================================================================
void RingBuffer::init(uint16_t size, uint8_t max_header_found) {
    // buffer initialization
    buffer_ = (uint8_t*)malloc(size);
    bzero(buffer_, size);

    // state initialization
    state_.capacity = size;
    state_.head = -1;
    state_.tail = 0;
    state_.count = 0;
    state_.state = RingBufferInitState::INITIALIZED;

    // search indices initialization
    index_.size = max_header_found;
    index_.pos = (uint16_t*)malloc(index_.size);
    index_.dist = (uint16_t*)malloc(index_.size);
    bzero(index_.pos, index_.size);
    bzero(index_.dist, index_.size);
    index_.count = 0;
    index_.searched = false;
}

// ============================================================================
void RingBuffer::reset() {
    std::lock_guard guard(mutex_);
    // buffer reset to zero
    bzero(buffer_, state_.capacity);
    // state reset
    state_.head = -1;
    state_.tail = 0;
    state_.count = 0;
    state_.state = RingBufferInitState::RESETTED;

    // search indices reset
    bzero(index_.pos, index_.size);
    bzero(index_.dist, index_.size);
    index_.count = 0;
    index_.searched = false;
}

// ============================================================================
bool RingBuffer::push(uint8_t data) {
    if ((int)state_.state <= 0) {
        return false;
    }
    // if the ringbuffer is just initialized or resetted
    if (state_.head == -1) {
        state_.head = 0;
    }

    buffer_[state_.tail] = data;
    state_.tail++;
    state_.tail = (int16_t)index((int16_t)state_.tail);

    state_.count++;
    if (state_.count > state_.capacity) {
        state_.count--;
        state_.head++;
        state_.head = index(state_.head);
    }

    return true;
}

// ============================================================================
bool RingBuffer::push(uint8_t* data, uint16_t len) {
    std::lock_guard guard(mutex_);
    if ((int)state_.state <= 0) {
        return false;
    }
    // FIXME: this is not an efficient operation
    for (uint16_t i = 0; i < len; i++) {
        if (!push(data[i])) {
            return false;
        }
    }

    return true;
}

// ============================================================================
bool RingBuffer::pop(uint8_t* ret) {
    if (state_.count <= 0) {
        return false;
    }
    *ret = buffer_[state_.head];
    // reset to zero
    buffer_[state_.head] = 0;
    state_.head++;
    state_.head = index(state_.head);
    state_.count--;
    return true;
}

// ============================================================================
bool RingBuffer::pop(uint8_t* ret, uint16_t len) {
    std::lock_guard guard(mutex_);
    // cannot get negative number of items, or try to get
    // more than count items
    if ((len < 1) || (len > state_.count)) {
        return false;
    }
    for (uint16_t i = 0; i < len; i++) {
        pop(&ret[i]);
    }
    return true;
}

// ============================================================================
/// set tail to a desired location, could only increase the count
/// will delete this function, do not use
bool RingBuffer::tail(uint16_t pos) {
    if (pos >= state_.capacity) {
        return false;
    }
    else if ((state_.head == state_.tail)
             && (state_.capacity == state_.count)) {
        state_.tail = pos;
        state_.head = (int16_t)pos;
    }
    else if (state_.head < state_.tail) {
        if (pos < state_.head) {
            state_.tail = pos;
            state_.count = (uint16_t)(state_.capacity - state_.head + pos);
        }
        else if (pos == state_.head) {
            state_.tail = pos;
            state_.count = state_.capacity;
        }
        else if ((pos > state_.head) && (pos < state_.tail)) {
            // should not make the count less
            return false;
        }
        else if (pos == state_.tail) {
            // no change?
        }
        else if (pos > state_.tail) {
            state_.count = (uint16_t)(state_.count + pos - state_.tail);
            state_.tail = pos;
        }
    }
    else if (state_.head > state_.tail) {
        if (pos < state_.tail) {
            // cannot decrease the count
            return false;
        }
        else if (pos == state_.tail) {
            // no change?
        }
        else if ((pos > state_.tail) && (pos < state_.head)) {
            state_.count = (uint16_t)(state_.count + pos - state_.tail);
            state_.tail = pos;
        }
        else if (pos == state_.head) {
            state_.tail = pos;
            state_.count = state_.capacity;
        }
        else if (pos > state_.head) {
            // cannot decrease the count
            return false;
        }
    }
    return true;
}

// ============================================================================
// UART DMA added a few bytes into the ringbuffer, so we just need to move tail
// ahead
bool RingBuffer::added(uint16_t count) {
    std::lock_guard guard(mutex_);
    state_.tail = (uint16_t)index((int16_t)(state_.tail + count));
    if (state_.head == state_.tail) {
        state_.head = (int16_t)state_.tail;
    }
    else if (count + state_.count >= state_.capacity) {
        state_.count = state_.capacity;
        state_.head = (int16_t)state_.tail;
    }
    else {
        state_.count = (uint16_t)(state_.count + count);
    }
    return true;
}

#define NOC "\033[0m"
#define HGRN "\033[1;32m"
#define HYLW "\033[1;33m"
#define HCYN "\033[1;36m"

// ============================================================================
void RingBuffer::show(char style, uint16_t width) {
    if ((int)state_.state <= 0) {
        printf("ringbuffer | uninitialized!\n");
        return;
    }
    printf("ringbuffer | %3d/%3d | ", state_.count, state_.capacity);
    if (state_.head == -1) {
        switch (state_.state) {
        case RingBufferInitState::INITIALIZED:
            printf("INITIALIZED\n");
            return;
        case RingBufferInitState::RESETTED:
            printf("RESETTED\n");
            return;
        }
    }
    // force to show at least 5 items in a row
    width = (width < 5) ? 5 : width;
    if (state_.head == state_.tail) {
        printf(HYLW "head & tail" NOC "\n");
    }
    else {
        printf(HGRN "head" NOC " | " HCYN "tail" NOC "\n");
    }
    for (int16_t i = 0; i < state_.capacity; i++) {
        if ((state_.head == i) && (state_.tail == i)) {
            printf(HYLW);
        }
        else if (state_.head == i) {
            printf(HGRN);
        }
        else if (state_.tail == i) {
            printf(HCYN);
        }
        switch (style) {
        case 'd':
        case 'D':
            printf("%3d  ", buffer_[i]);
            break;
        case 'h':
            printf("%02x  ", buffer_[i]);
            break;
        case 'H':
        default:
            printf("%02X  ", buffer_[i]);
            break;
        }
        if ((state_.head == i) || (state_.tail == i)) {
            printf(NOC);
        }
        if ((i + 1) % width == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

// ============================================================================
void RingBuffer::error(RingBufferError e) {
    if ((int)e > 0) {
        return;
    }
    std::basic_string_view<char> err_str = magic_enum::enum_name(e);
    std::cout << "RingBufferError: " << err_str << std::endl;
}

#define RINGBUFFER_HEADER_MAX_LEN 5

// ============================================================================
uint16_t RingBuffer::getPacketLen(uint16_t head_pos, uint8_t len_pos,
                                  uint8_t len_width) {

    uint16_t pos = (uint16_t)index((int16_t)(head_pos + len_pos));
    if (len_width == 1) {
        return (uint16_t)(buffer_[pos]);
    }
    else {
        uint8_t data1 = buffer_[pos];
        pos = (uint16_t)index((int16_t)(pos + 1));
        uint8_t data2 = buffer_[pos];
        return (uint16_t)((data2 << 8) + data1);
    }
}

// ============================================================================
/// always search from the head to the tail in a ringbuffer
/// this search is only designed for DJ's protocol use, not defined for other
/// devices
int8_t RingBuffer::search(uint8_t* header, uint8_t header_size, uint8_t len_pos,
                          uint8_t len_width) {
    std::lock_guard guard(mutex_);
    // header cannot be too small
    if (header_size <= 1) {
        return (int8_t)RingBufferError::HEADER_TOO_SMALL;
    }
    if (header_size > RINGBUFFER_HEADER_MAX_LEN) {
        return (int8_t)RingBufferError::HEADER_TOO_LARGE;
    }
    // ringbuffer is just initialized/resetted
    if (state_.head == -1) {
        return (int8_t)RingBufferError::JUST_INITIALIZED;
    }
    // ringbuffer does not have enough bytes (header + the length indicator)
    // FIXME: what if the length indicator is not just behind the header?
    if (state_.count < header_size + 2) {
        return (int8_t)RingBufferError::FEW_COUNT;
    }
    // make sure the [length] is just behind
    if ((len_pos < header_size) || (len_pos >= header_size + 2)) {
        return (int8_t)RingBufferError::LEN_POS_ERROR;
    }
    if (len_width > 2) {
        return (int8_t)RingBufferError::LEN_WIDTH_ERROR;
    }

    // mark it as searched
    index_.searched = true;
    // initialize the idx to match
    uint16_t idx[RINGBUFFER_HEADER_MAX_LEN] = { 0 };
    for (uint16_t i = 0; i < (uint16_t)header_size; i++) {
        idx[i] = (uint16_t)index((int16_t)(state_.head + i));
    }

    // start to search
    uint8_t search_i = 0;
    index_.count = 0;

    while (search_i < state_.count - header_size + 1) {
        // match test
        int8_t match_count = 0;
        for (int8_t i = 0; i < header_size; i++) {
            if (buffer_[idx[i]] != header[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }

        // record the position of found header
        if (match_count == header_size) {
            index_.pos[index_.count] = idx[0];
            index_.dist[index_.count] = 0;
            index_.count++;
            if (index_.count >= index_.size) {
                printf(HYLW "%s(): communication too heavy, need to expand the "
                            "ringbuffer or process it timely!\n" NOC,
                       __func__);
            }
        }
        // increase the check idx
        for (int i = 0; i < header_size - 1; i++) {
            idx[i] = idx[i + 1];
        }
        idx[header_size - 1] =
            (uint16_t)index((int16_t)(idx[header_size - 1] + 1));
        search_i++;
    }

    if (index_.count == 0) {
        return 0;
    }
    // if it has more bytes than [len] indicated, then it is a valid packet
    int8_t last_packet = 0;
    if (index_.count > 1) {
        for (int8_t i = 0; i < index_.count - 1; i++) {
            index_.dist[i] =
                (uint16_t)index((int16_t)(index_.pos[i + 1] - index_.pos[i]));
        }
        last_packet = (int8_t)(index_.count - 1);
    }
    if (index_.count == 1) {
        last_packet = 0;
    }
    // use len_pos and len_width to check if a header is valid
    uint16_t len = getPacketLen(index_.pos[last_packet], len_pos, len_width);
    if (state_.count >= (uint16_t)(header_size + 2 + len)) {
        index_.dist[last_packet] = (uint16_t)(header_size + 2 + len);
    }
    else {
        index_.count--;
    }
    return index_.count;
}

// ============================================================================
void RingBuffer::insight() {
    std::lock_guard guard(mutex_);
    if (index_.searched == false) {
        printf("ringbuffer is not searched.\n");
        return;
    }
    if (index_.count == 0) {
        printf("no header is found in ringbuffer.\n");
        return;
    }
    printf("found %d header(s) in ringbuffer:\n", index_.count);
    for (uint8_t i = 0; i < index_.count; i++) {
        printf("%3d (%3d)\n", index_.pos[i], index_.dist[i]);
    }
}

// ============================================================================
int8_t RingBuffer::fetch(uint8_t* array, uint16_t size) {
    std::lock_guard guard(mutex_);
    // not searched, or not found
    if (index_.count == 0) {
        return (int8_t)RingBufferError::FIND_NO_HEADER;
    }
    bzero(array, size);
    if (size < index_.dist[0]) {
        printf("%s() argument \"size\" too small\n", __func__);
        return (int8_t)RingBufferError::FETCH_DES_SMALL;
    }
    uint8_t popout;
    while (state_.head != index_.pos[0]) {
        pop(&popout);
    }
    pop(array, index_.dist[0]);
    index_.count--;
    for (int i = 0; i < index_.size - 1; i++) {
        index_.pos[i] = index_.pos[i + 1];
        index_.dist[i] = index_.dist[i + 1];
    }
    return index_.count;
}
