#include "ring-buffer.h"
#include <stdint.h>
#include <string.h>

// ============================================================================
#ifdef CONSOLE_IS_USED
#define rb_printf(...) (console.printf(__VA_ARGS__))
#define rb_printk(...) (console.printk(__VA_ARGS__))
#define rb_error(...) (console.error(__VA_ARGS__))
#else
#define rb_printf(...) ({ ; })
#define rb_printk(...) ({ ; })
#define rb_error(...) ({ ; })
#endif

typedef enum RingBufferInit {
    RINGBUFFER_INITIALIZED = 1,
    RINGBUFFER_RESETTED = 2,
    RINGBUFFER_EMPTIED = 3,
} RingBufferInit_e;

void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width);

// ============================================================================
static uint16_t RingBufferIndex(RingBuffer_t* rb, int idx) {
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
    rb.data = buffer;
    rb.state.capacity = size;
    rb.state.head = -1;
    rb.state.tail = 0;
    rb.state.count = 0;
    rb.state.is_initialized = RINGBUFFER_INITIALIZED;
    // data initialization
    for (uint16_t i = 0; i < size; i++) {
        rb.data[i] = 0;
    }
    return rb;
}

// ============================================================================
void RingBufferReset(RingBuffer_t* rb) {
    // data reset to zero
    for (uint16_t i = 0; i < rb->state.capacity; i++) {
        rb->data[i] = 0;
    }
    rb->state.head = -1;
    rb->state.tail = 0;
    rb->state.count = 0;
    rb->state.is_initialized = RINGBUFFER_RESETTED;
}

// ============================================================================
/// add one item into the ringbuffer at the tail position, and then move
/// tail forward for 1 position
bool RingBufferPush(RingBuffer_t* rb, uint8_t data) {
    if (rb->state.is_initialized <= 0) {
        return false;
    }
    // if the ringbuffer is just created
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
        rb->state.head = RingBufferIndex(rb, rb->state.head);
    }
    return true;
}

// ============================================================================
bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len) {
    if (rb->state.is_initialized <= 0) {
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
    // if it is the last byte to be popped out, move tail to a new
    if (rb->state.count == 0) {
        rb->state.head = -1;
        rb->state.tail = 0;
        rb->state.is_initialized = RINGBUFFER_EMPTIED;
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
    if (rb->state.is_initialized <= 0) {
        rb_printk(0, "ringbuffer | uninitialized!\r\n");
        return;
    }
    rb_printk(0, "ringbuffer | %3d/%3d | ", rb->state.count,
              rb->state.capacity);
    if (rb->state.head == -1) {
        switch (rb->state.is_initialized) {
        case RINGBUFFER_INITIALIZED:
            rb_printk(0, "INITIALIZED\r\n");
            return;
        case RINGBUFFER_RESETTED:
            rb_printk(0, "RESETTED\r\n");
            return;
        case RINGBUFFER_EMPTIED:
            rb_printk(0, "EMPTIED\r\n");
            return;
        }
    }
    // force to show at least 5 items in a row
    width = (width < 5) ? 5 : width;
    if (rb->state.head == rb->state.tail) {
        rb_printk(0, HYLW "head & tail" NOC "\r\n");
    }
    else {
        rb_printk(0, HGRN "head" NOC ", " HCYN "tail" NOC "\r\n");
    }
    // rb_printk(0, "--------------\r\n");
    for (int16_t i = 0; i < rb->state.capacity; i++) {
        if ((rb->state.head == i) && (rb->state.tail == i)) {
            rb_printk(0, HYLW);
        }
        else if (rb->state.head == i) {
            rb_printk(0, HGRN);
        }
        else if (rb->state.tail == i) {
            rb_printk(0, HCYN);
        }
        switch (style) {
        case 'd':
        case 'D':
            rb_printk(0, "%3d  ", rb->data[i]);
            break;
        case 'h':
        case 'H':
        default:
            rb_printk(0, "%02X  ", rb->data[i]);
            break;
        }
        if ((rb->state.head == i) || (rb->state.tail == i)) {
            rb_printk(0, NOC);
        }
        if ((i + 1) % width == 0) {
            rb_printk(0, "\r\n");
        }
    }
    rb_printk(0, "\r\n");
}

// ============================================================================
void RingBufferHeader(RingBuffer_t* rb, uint8_t* array, uint8_t size) {
    if (size > 5) {
        rb_error("%s(): size cannot be larger than 5\r\n");
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
// other cases: two bytes pattern, but three bytes shows two patterns
WARN_UNUSED_RESULT RingBufferError_e RingBufferSearch(RingBuffer_t* rb) {
    uint8_t size = rb->header.size;
    // ringbuffer does not have enough bytes
    if (rb->state.count < size) {
        return RINGBUFFER_ERR_SNCP;
    }
    // initialize the indices to match
    int indices[size];
    for (int i = 0; i < size; i++) {
        indices[i] = RingBufferIndex(rb, rb->state.head + i);
    }

    // start to search ----------------------
    int search_count = 0;
    rb->index.count = 0;
    while (search_count < rb->state.count - size + 1) {
        // match test --------
        int match_count = 0;
        for (int i = 0; i < size; i++) {
            if (rb->data[indices[i]] != rb->header.data[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }

        // record the position of found header -----------
        if (match_count == size) {
            rb->index.dist[rb->index.count] = 0;
            rb->index.pos[rb->index.count++] = indices[0];
            if (rb->index.count >= _RINGBUFFER_MAX_PACKETS_FOUND) {
                rb_printf(HYLW
                          "waring: communication too heavy, need to expand the "
                          "ringbuffer or process it timely!\r\n" NOC);
                RingBufferShow(rb, 'h', 9);
                while (1)
                    ;
            }
        }
        // counts the effective number of bytes in a (potential) packet -----
        if (rb->index.count > 0) {
            rb->index.dist[rb->index.count - 1]++;
        }

        // increase the check indices -----------
        for (int i = 0; i < size - 1; i++) {
            indices[i] = indices[i + 1];
        }
        indices[size - 1] = RingBufferIndex(rb, indices[size - 1] + 1);

        search_count++;
    }
    // the last one needs to add 1 -----------
    rb->index.dist[rb->index.count - 1]++;
    return RINGBUFFER_NO_ERROR;
}

// ============================================================================
/// move the head to specified position
RingBufferError_e RingBufferMoveHead(RingBuffer_t* rb, int16_t pos) {
    // cannot move head out of range
    if ((pos < 0) || (pos >= rb->state.capacity)) {
        return RINGBUFFER_ERR_OOR;
    }

    if (rb->state.head < rb->state.tail) {
        if ((pos >= rb->state.head) && (pos < rb->state.tail)) {
            rb->state.count -= (pos - rb->state.head);
            rb->state.head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else {
            return RINGBUFFER_ERR_OOR;
        }
    }
    else if (rb->state.head == rb->state.tail) {
        return RINGBUFFER_ERR_CNM;
    }
    else if (rb->state.head > rb->state.tail) {
        if (pos >= rb->state.head) {
            rb->state.count -= (pos - rb->state.head);
            rb->state.head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else if (pos < rb->state.tail) {
            rb->state.count -= (rb->state.capacity - rb->state.head) + pos;
            rb->state.head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else {  // out of range
            return RINGBUFFER_ERR_OOR;
        }
    }
    return RINGBUFFER_NO_ERROR;
}

// ============================================================================
void RingBufferInsight(RingBuffer_t* rb) {
    rb_printk(0, "--------\r\nfound %d header(s):\r\n", rb->index.count);
    for (uint8_t i = 0; i < rb->index.count; i++) {
        rb_printf("%3d (%3d)\r\n", rb->index.pos[i], rb->index.dist[i]);
    }
}
