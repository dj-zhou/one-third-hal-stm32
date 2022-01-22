#include "ringbuffer.h"

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
    state_.state = RINGBUFFER_INITIALIZED;

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
    state_.state = RINGBUFFER_INITIALIZED;

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
    // buffer reset to zero
    bzero(buffer_, state_.capacity);
    // state reset
    state_.head = -1;
    state_.tail = 0;
    state_.count = 0;
    state_.state = RINGBUFFER_RESETTED;

    // search indices reset
    bzero(index_.pos, index_.size);
    bzero(index_.dist, index_.size);
    index_.count = 0;
    index_.searched = false;
}

// ============================================================================
bool RingBuffer::push(uint8_t data) {
    if (state_.state <= 0) {
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
    if (state_.state <= 0) {
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
    // if the last byte is popped out, start from fresh
    if (state_.count == 0) {
        state_.head = -1;
        state_.tail = 0;
        state_.state = RINGBUFFER_EMPTIED;
        index_.searched = false;
    }
    return true;
}

// ============================================================================
bool RingBuffer::pop(uint8_t* ret, uint16_t len) {
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
    if (state_.state <= 0) {
        printf("ringbuffer | uninitialized!\r\n");
        return;
    }
    printf("ringbuffer | %3d/%3d | ", state_.count, state_.capacity);
    if (state_.head == -1) {
        switch (state_.state) {
        case RINGBUFFER_INITIALIZED:
            printf("INITIALIZED\r\n");
            return;
        case RINGBUFFER_RESETTED:
            printf("RESETTED\r\n");
            return;
        case RINGBUFFER_EMPTIED:
            printf("EMPTIED\r\n");
            return;
        }
    }
    // force to show at least 5 items in a row
    width = (width < 5) ? 5 : width;
    if (state_.head == state_.tail) {
        printf(HYLW "head & tail" NOC "\r\n");
    }
    else {
        printf(HGRN "head" NOC ", " HCYN "tail" NOC "\r\n");
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
        case 'H':
        default:
            printf("%02X  ", buffer_[i]);
            break;
        }
        if ((state_.head == i) || (state_.tail == i)) {
            printf(NOC);
        }
        if ((i + 1) % width == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

// ============================================================================
void RingBuffer::error(RingBufferError_e e) {
    if (e > 0) {
        return;
    }
    printf("RingBufferError: ");
    switch (e) {
    case RINGBUFFER_HEADER_TOO_SMALL:
        printf("header too small\r\n");
        break;
    case RINGBUFFER_HEADER_TOO_LARGE:
        printf("header too large\r\n");
        break;
    case RINGBUFFER_JUST_INITIALIZED:
        printf("just initialized\r\n");
        break;
    case RINGBUFFER_FEW_COUNT:
        printf("few count\r\n");
        break;
    case RINGBUFFER_FIND_NO_HEADER:
        printf("find no header\r\n");
        break;
    case RINGBUFFER_FETCH_DES_SMALL:
        printf("fetch destination small\r\n");
        break;
    default:
        break;
    }
}

#define RINGBUFFER_HEADER_MAX_LEN 5
// ============================================================================
/// always search from the head to the tail in a ringbuffer
int8_t RingBuffer::search(uint8_t* header, uint8_t header_size) {
    // header is not assigned
    if (header_size <= 1) {
        return (int8_t)RINGBUFFER_HEADER_TOO_SMALL;
    }
    if (header_size > RINGBUFFER_HEADER_MAX_LEN) {
        return (int8_t)RINGBUFFER_HEADER_TOO_LARGE;
    }
    // ringbuffer is just initialized/resetted, or emptied
    if (state_.head == -1) {
        return (int8_t)RINGBUFFER_JUST_INITIALIZED;
    }
    // mark it as searched
    index_.searched = true;

    // ringbuffer does not have enough bytes
    if (state_.count < header_size) {
        return (int8_t)RINGBUFFER_FEW_COUNT;
    }
    // initialize the indices to match
    // not a good way to initialize
    uint16_t indices[RINGBUFFER_HEADER_MAX_LEN];
    for (uint16_t i = 0; i < (uint16_t)header_size; i++) {
        // bug: after initialization, state.head = -1 == 65536
        indices[i] = (uint16_t)index((int16_t)(state_.head + i));
    }

    // start to search
    uint8_t search_count = 0;
    index_.count = 0;
    while (search_count < state_.count - header_size + 1) {
        // match test
        int match_count = 0;
        for (int i = 0; i < header_size; i++) {
            if (buffer_[indices[i]] != header[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }

        // record the position of found header
        if (match_count == header_size) {
            index_.dist[index_.count] = 0;
            index_.pos[index_.count++] = indices[0];
            if (index_.count >= index_.size) {
                printf(HYLW "%s(): communication too heavy, need to expand the "
                            "ringbuffer or process it timely!\r\n" NOC,
                       __func__);
                // do not block here
            }
        }
        // counts the effective number of bytes in a (potential) packet
        if (index_.count > 0) {
            index_.dist[index_.count - 1]++;
        }

        // increase the check indices
        for (int i = 0; i < header_size - 1; i++) {
            indices[i] = indices[i + 1];
        }
        indices[header_size - 1] =
            (uint16_t)index((int16_t)(indices[header_size - 1] + 1));
        search_count++;
    }
    // the last one needs to add 1
    index_.dist[index_.count - 1]++;
    return index_.count;
}

// ============================================================================
void RingBuffer::insight() {
    if (index_.searched == false) {
        printf("ringbuffer is not searched.\r\n");
        return;
    }
    if (index_.count == 0) {
        printf("no header is found in ringbuffer.\r\n");
        return;
    }
    printf("found %d header(s) in ringbuffer:\r\n", index_.count);
    for (uint8_t i = 0; i < index_.count; i++) {
        printf("%3d (%3d)\r\n", index_.pos[i], index_.dist[i]);
    }
}

// ============================================================================
int8_t RingBuffer::fetch(uint8_t* array, uint16_t size) {
    // not searched, or not found
    if (index_.count == 0) {
        return (int8_t)RINGBUFFER_FIND_NO_HEADER;
    }
    bzero(array, size);
    if (size < index_.dist[0]) {
        printf("%s() argument \"size\" too small\r\n", __func__);
        return (int8_t)RINGBUFFER_FETCH_DES_SMALL;
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
