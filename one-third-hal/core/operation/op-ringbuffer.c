#include "config.h"

#include "operation.h"
#include <string.h>

// ============================================================================
static uint16_t RingBufferIndex(RingBuffer_t* rb, uint16_t idx) {
    while (idx >= rb->state.capacity) {
        idx -= rb->state.capacity;
    }
    while (idx < 0) {
        idx += rb->state.capacity;
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

    // header initialization
    bzero(rb.header.data, _RINGBUFFER_HEADER_MAX_LEN);
    rb.header.size = 0;

    // search indices initialization
    bzero(rb.index.pos, _RINGBUFFER_PACKETS_MAX_FOUND);
    bzero(rb.index.dist, _RINGBUFFER_PACKETS_MAX_FOUND);
    rb.index.count = 0;
    rb.index.searched = false;

    return rb;
}

// ============================================================================
void RingBufferReset(RingBuffer_t* rb) {
    // data reset to zero
    for (uint16_t i = 0; i < rb->state.capacity; i++) {
        rb->data[i] = 0;
    }
    // state reset
    rb->state.head = -1;
    rb->state.tail = 0;
    rb->state.count = 0;
    rb->state.state = RINGBUFFER_RESETTED;

    // header reset
    bzero(rb->header.data, _RINGBUFFER_HEADER_MAX_LEN);
    rb->header.size = 0;

    // search indices reset
    bzero(rb->index.pos, _RINGBUFFER_PACKETS_MAX_FOUND);
    bzero(rb->index.dist, _RINGBUFFER_PACKETS_MAX_FOUND);
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
    rb->state.tail = RingBufferIndex(rb, rb->state.tail);

    rb->state.count++;
    if (rb->state.count > rb->state.capacity) {
        rb->state.count--;
        rb->state.head++;
        rb->state.head =
            ( int16_t )RingBufferIndex(rb, ( uint16_t )rb->state.head);
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
    rb->state.head = ( int16_t )RingBufferIndex(rb, ( uint16_t )rb->state.head);
    rb->state.count--;
    // if it is the last byte to be popped out, move tail to a new
    if (rb->state.count == 0) {
        rb->state.head = -1;
        rb->state.tail = 0;
        rb->state.state = RINGBUFFER_EMPTIED;
    }
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
void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width) {
    if (rb->state.state <= 0) {
        ringbuffer_printk(0, "ringbuffer | uninitialized!\r\n");
        return;
    }
    ringbuffer_printk(0, "ringbuffer | %3d/%3d | ", rb->state.count,
                      rb->state.capacity);
    if (rb->state.head == -1) {
        switch (rb->state.state) {
        case RINGBUFFER_INITIALIZED:
            ringbuffer_printk(0, "INITIALIZED\r\n");
            return;
        case RINGBUFFER_RESETTED:
            ringbuffer_printk(0, "RESETTED\r\n");
            return;
        case RINGBUFFER_EMPTIED:
            ringbuffer_printk(0, "EMPTIED\r\n");
            return;
        }
    }
    // force to show at least 5 items in a row
    width = (width < 5) ? 5 : width;
    if (rb->state.head == rb->state.tail) {
        ringbuffer_printk(0, HYLW "head & tail" NOC "\r\n");
    }
    else {
        ringbuffer_printk(0, HGRN "head" NOC ", " HCYN "tail" NOC "\r\n");
    }
    for (int16_t i = 0; i < rb->state.capacity; i++) {
        if ((rb->state.head == i) && (rb->state.tail == i)) {
            ringbuffer_printk(0, HYLW);
        }
        else if (rb->state.head == i) {
            ringbuffer_printk(0, HGRN);
        }
        else if (rb->state.tail == i) {
            ringbuffer_printk(0, HCYN);
        }
        switch (style) {
        case 'd':
        case 'D':
            ringbuffer_printk(0, "%3d  ", rb->data[i]);
            break;
        case 'h':
        case 'H':
        default:
            ringbuffer_printk(0, "%02X  ", rb->data[i]);
            break;
        }
        if ((rb->state.head == i) || (rb->state.tail == i)) {
            ringbuffer_printk(0, NOC);
        }
        if ((i + 1) % width == 0) {
            ringbuffer_printk(0, "\r\n");
        }
    }
    ringbuffer_printk(0, "\r\n");
}

// ============================================================================
void RingBufferHeader(RingBuffer_t* rb, uint8_t* array, uint8_t size) {
    if (size > 5) {
        ringbuffer_error("%s(): size cannot be larger than 5\r\n");
    }
    uint8_t* ptr = rb->header.data;
    for (uint8_t i = 0; i < size; i++) {
        *ptr++ = array[i];
    }
    if (size < 5) {
        for (uint8_t i = 0; i < 5 - size; i++) {
            *ptr++ = 0;
        }
    }
    rb->header.size = size;
}

// ============================================================================
/// always search from the head to the tail of a ringbuffer
/// other cases to fix: two bytes pattern, but three bytes shows two patterns
WARN_UNUSED_RESULT int8_t RingBufferSearch(RingBuffer_t* rb) {
    if (rb->header.size == 0) {
        ringbuffer_error("ringbuffer header need to be assigned by "
                         "op.ringbuffer.header()\r\n");
    }
    uint8_t size = rb->header.size;
    // mark it as searched
    rb->index.searched = true;
    // ringbuffer does not have enough bytes
    if (rb->state.count < size) {
        return 0;
    }
    // initialize the indices to match
    uint16_t indices[size];
    for (uint16_t i = 0; i < ( uint16_t )size; i++) {
        indices[i] = RingBufferIndex(rb, ( uint16_t )rb->state.head + i);
    }

    // start to search
    uint8_t search_count = 0;
    rb->index.count = 0;
    while (search_count < rb->state.count - size + 1) {
        // match test
        int match_count = 0;
        for (int i = 0; i < size; i++) {
            if (rb->data[indices[i]] != rb->header.data[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }

        // record the position of found header
        if (match_count == size) {
            rb->index.dist[rb->index.count] = 0;
            rb->index.pos[rb->index.count++] = indices[0];
            if (rb->index.count >= _RINGBUFFER_PACKETS_MAX_FOUND) {
                ringbuffer_printf(HYLW
                                  "communication too heavy, need to expand the "
                                  "ringbuffer or process it timely!\r\n" NOC);
                // do not block here
            }
        }
        // counts the effective number of bytes in a (potential) packet
        if (rb->index.count > 0) {
            rb->index.dist[rb->index.count - 1]++;
        }

        // increase the check indices
        for (int i = 0; i < size - 1; i++) {
            indices[i] = indices[i + 1];
        }
        indices[size - 1] = RingBufferIndex(rb, indices[size - 1] + 1);
        search_count++;
    }
    // the last one needs to add 1
    rb->index.dist[rb->index.count - 1]++;
    return rb->index.count;
}

// ============================================================================
void RingBufferInsight(RingBuffer_t* rb) {
    if (rb->index.searched == false) {
        ringbuffer_printk(0, "ringbuffer is not searched.\r\n");
        return;
    }
    if (rb->index.count == 0) {
        ringbuffer_printk(0, "no header is found in ringbuffer.\r\n");
        return;
    }
    ringbuffer_printk(0, "found %d header(s) in ringbuffer:\r\n",
                      rb->index.count);
    for (uint8_t i = 0; i < rb->index.count; i++) {
        ringbuffer_printf("%3d (%3d)\r\n", rb->index.pos[i], rb->index.dist[i]);
    }
}

// ============================================================================
WARN_UNUSED_RESULT int8_t RingBufferFetch(RingBuffer_t* rb, uint8_t* array,
                                          uint16_t size) {
    if (rb->index.count == 0) {
        return -1;
    }
    bzero(array, size);
    if (size < rb->index.dist[0]) {
        ringbuffer_printf("%s() argument 3: size is too small\r\n", __func__);
        return -2;
    }
    uint8_t popout;
    if (rb->state.head != rb->index.pos[0]) {
        for (uint16_t i = 0; i < rb->index.pos[0] - rb->state.head; i++) {
            RingBufferPop(rb, &popout);
        }
    }
    RingBufferPopN(rb, array, rb->index.dist[0]);
    rb->index.count--;
    for (int i = 0; i < _RINGBUFFER_PACKETS_MAX_FOUND - 1; i++) {
        rb->index.pos[i] = rb->index.pos[i + 1];
        rb->index.dist[i] = rb->index.dist[i + 1];
    }
    return rb->index.count;
}
