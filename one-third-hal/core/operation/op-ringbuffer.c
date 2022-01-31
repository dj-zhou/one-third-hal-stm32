#include "config.h"

#include "operation.h"
#include <string.h>

// ============================================================================
#if defined(CONSOLE_IS_USED)
#define ringbuffer_printf(...) (console.printf(__VA_ARGS__))
#define ringbuffer_printk(...) (console.printk(__VA_ARGS__))
#define ringbuffer_error(...) (console.error(__VA_ARGS__))
#else
#define ringbuffer_printf(...) ({ ; })
#define ringbuffer_printk(...) ({ ; })
#define ringbuffer_error(...) ({ ; })
#endif

// ============================================================================
static int16_t RingBufferIndex(RingBuffer_t* rb, int16_t idx) {
    while (idx >= rb->state.capacity) {
        idx = (int16_t)(idx - rb->state.capacity);
    }
    while (idx < 0) {
        idx = (int16_t)(idx + rb->state.capacity);
    }
    return idx;
}

// ============================================================================
/// the RingBufferApi_t only defines the interface of using the ring-buffer, the
/// data block is defined outside and trasmitted into this function for
/// configuration
WARN_UNUSED_RESULT RingBuffer_t RingBufferInit(uint8_t* buffer, uint16_t size) {
    RingBuffer_t rb;
    // data initialization
    rb.data = buffer;
    bzero(rb.data, size);

    // state initialization
    rb.state.capacity = size;
    rb.state.head = -1;
    rb.state.tail = 0;
    rb.state.count = 0;
    rb.state.state = RINGBUFFER_INITIALIZED;

    // search idx initialization
    bzero(rb.index.pos, RINGBUFFER_PACKETS_MAX_FOUND);
    bzero(rb.index.dist, RINGBUFFER_PACKETS_MAX_FOUND);
    rb.index.count = 0;
    rb.index.searched = false;

    return rb;
}

// ============================================================================
void RingBufferReset(RingBuffer_t* rb) {
    // data reset to zero
    bzero(rb->data, rb->state.capacity);
    // state reset
    rb->state.head = -1;
    rb->state.tail = 0;
    rb->state.count = 0;
    rb->state.state = RINGBUFFER_RESETTED;

    // search idx reset
    bzero(rb->index.pos, RINGBUFFER_PACKETS_MAX_FOUND);
    bzero(rb->index.dist, RINGBUFFER_PACKETS_MAX_FOUND);
    rb->index.count = 0;
    rb->index.searched = false;
}

// ============================================================================
/// add one item into the ringbuffer at the tail position, and then move
/// tail forward for 1 position
bool RingBufferPush(RingBuffer_t* rb, uint8_t data) {
    if (rb->state.state <= 0) {
        return false;
    }
    // if the ringbuffer is just initialized or resetted
    if (rb->state.head == -1) {
        rb->state.head = 0;
    }

    rb->data[rb->state.tail] = data;
    rb->state.tail++;
    rb->state.tail = (uint16_t)RingBufferIndex(rb, (int16_t)rb->state.tail);

    rb->state.count++;
    if (rb->state.count > rb->state.capacity) {
        rb->state.count--;
        rb->state.head++;
        rb->state.head = RingBufferIndex(rb, rb->state.head);
    }

    return true;
}

// ============================================================================
bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len) {
    if (rb->state.state <= 0) {
        return false;
    }
    // FIXME: this is not an efficient operation
    for (uint16_t i = 0; i < len; i++) {
        RingBufferPush(rb, data[i]);
    }

    return true;
}

// ============================================================================
/// Pop one item from the head, and move the head forward for one position.
/// If count == 0, means all data is get out, then return false, the head
/// pointer will not move forward
bool RingBufferPop(RingBuffer_t* rb, uint8_t* ret) {
    if (rb->state.count <= 0) {
        return false;
    }
    *ret = rb->data[rb->state.head];
    // reset to zero
    rb->data[rb->state.head] = 0;
    rb->state.head++;
    rb->state.head = RingBufferIndex(rb, rb->state.head);
    rb->state.count--;
    return true;
}

// ============================================================================
/// Pop N items out from the ringbuffer, starting from the head position. It
/// calls the RingBufferPop() function. However, if you want to get more items
/// than count items from the ringbuffer, it will return error
bool RingBufferPopN(RingBuffer_t* rb, uint8_t* ret, uint16_t len) {
    // cannot get negative number of items, or try to get
    // more than count items
    if ((len < 1) || (len > rb->state.count)) {
        return false;
    }
    for (uint16_t i = 0; i < len; i++) {
        RingBufferPop(rb, &ret[i]);
    }
    return true;
}

// ============================================================================
/// set tail to a desired location, could only increase the count
/// will delete this function, do not use
bool RingBufferTail(RingBuffer_t* rb, uint16_t pos) {
    if (pos >= rb->state.capacity) {
        return false;
    }
    else if ((rb->state.head == rb->state.tail)
             && (rb->state.capacity == rb->state.count)) {
        rb->state.tail = pos;
        rb->state.head = (int16_t)pos;
    }
    else if (rb->state.head < rb->state.tail) {
        if (pos < rb->state.head) {
            rb->state.tail = pos;
            rb->state.count =
                (uint16_t)(rb->state.capacity - rb->state.head + pos);
        }
        else if (pos == rb->state.head) {
            rb->state.tail = pos;
            rb->state.count = rb->state.capacity;
        }
        else if ((pos > rb->state.head) && (pos < rb->state.tail)) {
            // should not make the count less
            return false;
        }
        else if (pos == rb->state.tail) {
            // no change?
        }
        else if (pos > rb->state.tail) {
            rb->state.count =
                (uint16_t)(rb->state.count + pos - rb->state.tail);
            rb->state.tail = pos;
        }
    }
    else if (rb->state.head > rb->state.tail) {
        if (pos < rb->state.tail) {
            // cannot decrease the count
            return false;
        }
        else if (pos == rb->state.tail) {
            // no change?
        }
        else if ((pos > rb->state.tail) && (pos < rb->state.head)) {
            rb->state.count =
                (uint16_t)(rb->state.count + pos - rb->state.tail);
            rb->state.tail = pos;
        }
        else if (pos == rb->state.head) {
            rb->state.tail = pos;
            rb->state.count = rb->state.capacity;
        }
        else if (pos > rb->state.head) {
            // cannot decrease the count
            return false;
        }
    }
    return true;
}

// ============================================================================
// UART DMA added a few bytes into the ringbuffer, so we just need to move tail
// ahead
bool RingBufferAdded(RingBuffer_t* rb, uint16_t count) {
    rb->state.tail =
        (uint16_t)RingBufferIndex(rb, (int16_t)(rb->state.tail + count));
    if (rb->state.head == rb->state.tail) {
        rb->state.head = (int16_t)rb->state.tail;
    }
    else if (count + rb->state.count >= rb->state.capacity) {
        rb->state.count = rb->state.capacity;
        rb->state.head = (int16_t)rb->state.tail;
    }
    else {
        rb->state.count = (uint16_t)(rb->state.count + count);
    }
    return true;
}

// ============================================================================
void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width) {
    if (rb->state.state <= 0) {
        ringbuffer_printk(2, "ringbuffer | uninitialized!\r\n");
        return;
    }
    ringbuffer_printk(2, "ringbuffer | %3d/%3d | ", rb->state.count,
                      rb->state.capacity);
    if (rb->state.head == -1) {
        switch (rb->state.state) {
        case RINGBUFFER_INITIALIZED:
            ringbuffer_printk(2, "INITIALIZED\r\n");
            return;
        case RINGBUFFER_RESETTED:
            ringbuffer_printk(2, "RESETTED\r\n");
            return;
        }
    }
    // force to show at least 5 items in a row
    width = (width < 5) ? 5 : width;
    if (rb->state.head == rb->state.tail) {
        ringbuffer_printk(2, HYLW "head & tail" NOC "\r\n");
    }
    else {
        ringbuffer_printk(2, HGRN "head" NOC " | " HCYN "tail" NOC "\r\n");
    }
    for (int16_t i = 0; i < rb->state.capacity; i++) {
        if ((rb->state.head == i) && (rb->state.tail == i)) {
            ringbuffer_printk(2, HYLW);
        }
        else if (rb->state.head == i) {
            ringbuffer_printk(2, HGRN);
        }
        else if (rb->state.tail == i) {
            ringbuffer_printk(2, HCYN);
        }
        switch (style) {
        case 'd':
        case 'D':
            ringbuffer_printk(2, "%3d  ", rb->data[i]);
            break;
        case 'h':
            ringbuffer_printk(2, "%02x  ", rb->data[i]);
            break;
        case 'H':
        default:
            ringbuffer_printk(2, "%02X  ", rb->data[i]);
            break;
        }
        if ((rb->state.head == i) || (rb->state.tail == i)) {
            ringbuffer_printk(2, NOC);
        }
        if ((i + 1) % width == 0) {
            ringbuffer_printk(2, "\r\n");
        }
    }
    ringbuffer_printk(2, "\r\n");
}

// ============================================================================
void RingBufferError(RingBufferError_e e) {
    if (e > 0) {
        return;
    }
    ringbuffer_printf("RingBufferError: ");
    switch (e) {
    case RINGBUFFER_HEADER_TOO_SMALL:
        ringbuffer_printf("header too small\r\n");
        break;
    case RINGBUFFER_HEADER_TOO_LARGE:
        ringbuffer_printf("header too large\r\n");
        break;
    case RINGBUFFER_JUST_INITIALIZED:
        ringbuffer_printf("just initialized\r\n");
        break;
    case RINGBUFFER_FEW_COUNT:
        ringbuffer_printf("few count\r\n");
        break;
    case RINGBUFFER_FIND_NO_HEADER:
        ringbuffer_printf("find no header\r\n");
        break;
    case RINGBUFFER_FETCH_DES_SMALL:
        ringbuffer_printf("fetch destination small\r\n");
        break;
    case RINGBUFFER_LEN_POS_ERROR:
        ringbuffer_printf("len position error\r\n");
        break;
    case RINGBUFFER_LEN_WIDTH_ERROR:
        ringbuffer_printf("len width error\r\n");
    default:
        break;
    }
}

// ============================================================================
static uint16_t RingBufferGetPacketLen(RingBuffer_t* rb, uint16_t head_pos,
                                       uint8_t len_pos_or_type,
                                       uint8_t len_width) {
    if (len_width != 0) {
        uint8_t len_pos = len_pos_or_type;
        uint16_t pos =
            (uint16_t)RingBufferIndex(rb, (int16_t)(head_pos + len_pos));
        if (len_width == 1) {
            return (uint16_t)(rb->data[pos]);
        }
        else {
            uint8_t data1 = rb->data[pos];
            pos = (uint16_t)RingBufferIndex(rb, (int16_t)(pos + 1));
            uint8_t data2 = rb->data[pos];
            return (uint16_t)((data2 << 8) + data1);
        }
    }
    else {
        switch (len_pos_or_type) {
        case RINGBUFFER_SEARCH_TFMINI:
            return 5;
        case RINGBUFFER_SEARCH_MTI2:
            ringbuffer_printf("RINGBUFFER_SEARCH_MTI2: todo\r\n");
            return 0;
        default:
            return 0;
        }
    }
}

// ============================================================================
/// always search from the head to the tail in a ringbuffer
/// len_pos is the position in a protocol, must be 1 or 2 bytes
/// if len_width !=0, we use len_pos_or_type for len_pos and to calculate the
/// length of a packet, otherwise, we use len_pos_or_type for a search type
WARN_UNUSED_RESULT int8_t RingBufferSearch(RingBuffer_t* rb, uint8_t* header,
                                           uint8_t header_size,
                                           uint8_t len_pos_or_type,
                                           uint8_t len_width) {

    // header cannot be too small
    if (header_size <= 1) {
        return (int8_t)RINGBUFFER_HEADER_TOO_SMALL;
    }
    if (header_size > RINGBUFFER_HEADER_MAX_LEN) {
        return (int8_t)RINGBUFFER_HEADER_TOO_LARGE;
    }
    // ringbuffer is just initialized/resetted
    if (rb->state.head == -1) {
        return (int8_t)RINGBUFFER_JUST_INITIALIZED;
    }
    // ringbuffer does not have enough bytes (header + the length indicator)
    // FIXME: what if the length indicator is not just behind the header?
    if (rb->state.count < header_size + 2) {
        return (int8_t)RINGBUFFER_FEW_COUNT;
    }
    // make sure the [length] is just behind
    if (len_width != 0) {
        uint8_t len_pos = len_pos_or_type;
        if ((len_pos < header_size) || (len_pos >= header_size + 2)) {
            return (int8_t)RINGBUFFER_LEN_POS_ERROR;
        }
        if (len_width > 2) {
            return (int8_t)RINGBUFFER_LEN_WIDTH_ERROR;
        }
    }

    // mark it as searched
    rb->index.searched = true;
    // initialize the idx to match
    uint16_t idx[RINGBUFFER_HEADER_MAX_LEN] = { 0 };
    for (uint16_t i = 0; i < (uint16_t)header_size; i++) {
        idx[i] = (uint16_t)RingBufferIndex(rb, (int16_t)(rb->state.head + i));
    }

    // start to search
    uint8_t search_i = 0;
    rb->index.count = 0;

    while (search_i < rb->state.count - header_size + 1) {
        // match test
        uint8_t match_count = 0;
        for (uint8_t i = 0; i < header_size; i++) {
            if (rb->data[idx[i]] != header[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }
        // record the position of found header
        if (match_count == header_size) {
            rb->index.pos[rb->index.count] = idx[0];
            rb->index.dist[rb->index.count] = 0;
            rb->index.count++;
            if (rb->index.count >= RINGBUFFER_PACKETS_MAX_FOUND) {
                ringbuffer_printf(
                    HYLW "%s(): communication too heavy, need to expand the "
                         "ringbuffer or process it timely!\r\n" NOC,
                    __func__);
            }
        }
        // increase the check idx
        for (int i = 0; i < header_size - 1; i++) {
            idx[i] = idx[i + 1];
        }
        idx[header_size - 1] =
            (uint16_t)RingBufferIndex(rb, (int16_t)(idx[header_size - 1] + 1));
        search_i++;
    }

    if (rb->index.count == 0) {
        return 0;
    }
    // if it has more bytes than [len] indicated, then it is a valid packet
    int8_t last_packet = 0;
    if (rb->index.count > 1) {
        for (int8_t i = 0; i < rb->index.count - 1; i++) {
            rb->index.dist[i] = (uint16_t)RingBufferIndex(
                rb, (int16_t)(rb->index.pos[i + 1] - rb->index.pos[i]));
        }
        last_packet = (int8_t)(rb->index.count - 1);
    }
    if (rb->index.count == 1) {
        last_packet = 0;
    }
    // use len_pos and len_width to check if a header is valid
    uint16_t len = RingBufferGetPacketLen(rb, rb->index.pos[last_packet],
                                          len_pos_or_type, len_width);

    if (rb->state.count >= (uint16_t)(header_size + 2 + len)) {
        rb->index.dist[last_packet] = (uint16_t)(header_size + 2 + len);
    }
    else {
        rb->index.count--;
    }
    return rb->index.count;
}

// ============================================================================
void RingBufferInsight(RingBuffer_t* rb) {
    if (rb->index.searched == false) {
        ringbuffer_printk(2, "ringbuffer is not searched.\r\n");
        return;
    }
    if (rb->index.count == 0) {
        ringbuffer_printk(2, "no header is found in ringbuffer.\r\n");
        return;
    }
    ringbuffer_printk(2, "found %d header(s) in ringbuffer:\r\n",
                      rb->index.count);
    for (uint8_t i = 0; i < rb->index.count; i++) {
        ringbuffer_printf("%3d (%3d)\r\n", rb->index.pos[i], rb->index.dist[i]);
    }
}

// ============================================================================
WARN_UNUSED_RESULT int8_t RingBufferFetch(RingBuffer_t* rb, uint8_t* array,
                                          uint16_t size) {
    // not searched, or not found
    if (rb->index.count == 0) {
        return (int8_t)RINGBUFFER_FIND_NO_HEADER;
    }
    bzero(array, size);
    if (size < rb->index.dist[0]) {
        ringbuffer_printf("%s() argument \"size\" too small\r\n", __func__);
        return (int8_t)RINGBUFFER_FETCH_DES_SMALL;
    }
    uint8_t popout;
    while (rb->state.head != rb->index.pos[0]) {
        RingBufferPop(rb, &popout);
    }
    RingBufferPopN(rb, array, rb->index.dist[0]);
    rb->index.count--;
    for (int i = 0; i < RINGBUFFER_PACKETS_MAX_FOUND - 1; i++) {
        rb->index.pos[i] = rb->index.pos[i + 1];
        rb->index.dist[i] = rb->index.dist[i + 1];
    }
    return rb->index.count;
}
