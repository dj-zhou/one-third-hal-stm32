#include "ring-buffer.h"
#include <stdint.h>

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

static void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width);

// ============================================================================
static uint16_t RingBufferIndex(RingBuffer_t* rb, int idx) {
    while (idx >= rb->capacity) {
        idx -= rb->capacity;
    }
    while (idx < 0) {
        idx += rb->capacity;
    }
    return idx;
}

// ============================================================================
static RingBuffer_t RingBufferConfig(uint8_t* data, uint16_t size) {
    RingBuffer_t rb;
    rb.buffer      = data;
    rb.capacity    = size;
    rb.head        = -1;
    rb.tail        = 0;
    rb.count       = 0;
    rb.initialized = true;
    return rb;
}

// ============================================================================
/// add one item into the ringbuffer at the tail position, and then move
/// tail forward for 1 position
static bool RingBufferPush(RingBuffer_t* rb, uint8_t data) {
    if (!rb->initialized) {
        return false;
    }
    // if the ringbuffer is just created
    if (rb->head == -1) {
        rb->head = 0;
    }

    rb->buffer[rb->tail] = data;
    rb->tail++;
    rb->tail = RingBufferIndex(rb, rb->tail);

    rb->count++;
    if (rb->count > rb->capacity) {
        rb->count--;
        rb->head++;
        rb->head = RingBufferIndex(rb, rb->head);
    }
    return true;
}

// ============================================================================
static bool RingBufferPushN(RingBuffer_t* rb, uint8_t* data, uint16_t len) {
    if (!rb->initialized) {
        return false;
    }
    for (uint16_t i = 0; i < len; i++) {
        RingBufferPush(rb, data[i]);
    }
    return true;
}

// ============================================================================
/// Pop one item from the head, and move the head forward for one position.
/// If count == 0, means all data is get out, then return false, the head
/// pointer will not move forward
static bool RingBufferPop(RingBuffer_t* rb, uint8_t* ret) {
    if (rb->count < 0) {
        return false;
    }
    *ret = rb->buffer[rb->head];
    // reset to zero
    rb->buffer[rb->head] = 0;
    rb->head++;
    rb->head = RingBufferIndex(rb, rb->head);
    rb->count--;
    return true;
}

// ============================================================================
/// Pop N items out from the ringbuffer, starting from the head position. It
/// calls the RingBufferPop() function. However, if you want to get more items
/// than count items from the ringbuffer, it will return error
static bool RingBufferPopN(RingBuffer_t* rb, uint8_t* ret, uint16_t len) {
    // cannot get negative number of items, or try to get
    // more than count items
    if ((len < 1) || (len > rb->count)) {
        return false;
    }
    for (uint16_t i = 0; i < len; i++) {
        RingBufferPop(rb, &ret[i]);
    }
    return true;
}

// ============================================================================
/// always search from the head to the tail of a ringbuffer
// other cases: two bytes pattern, but three bytes shows two patterns
static RingBufferError_e RingBufferSearch(RingBuffer_t* rb, uint8_t* pattern,
                                          uint8_t            len,
                                          RingBufferIndex_t* index) {
    if (len < 2) {
        return RINGBUFFER_ERR_SPS;  // the pattern must be larger than 1
    }
    if (rb->count < len) {
        return RINGBUFFER_ERR_SNCP;  // ringbuffer does not have enough bytes
    }
    // initialize the indices to match
    int indices[len];
    for (int i = 0; i < len; i++) {
        indices[i] = RingBufferIndex(rb, rb->head + i);
    }

    // start to search ----------------------
    int search_count = 0;
    index->found     = 0;
    while (search_count < rb->count - len + 1) {
        // match test --------
        int match_count = 0;
        for (int i = 0; i < len; i++) {
            if (rb->buffer[indices[i]] != pattern[i]) {
                break;  // break the for loop
            }
            else {
                match_count++;
            }
        }

        // record the position of found pattern -----------
        if (match_count == len) {
            index->count[index->found] = 0;
            index->pos[index->found++] = indices[0];
            if (index->found >= _RINGBUFFER_MAX_PATTERN_FOUND) {
                rb_printf("waring: communication too heavy!\r\n");
                // debug purpose:
                RingBufferShow(rb, 'h', 9);
                while (1)
                    ;
            }
        }
        // counts the effective number of bytes in a (potential) packet -----
        if (index->found > 0) {
            index->count[index->found - 1]++;
        }

        // increase the check indices -----------
        for (int i = 0; i < len - 1; i++) {
            indices[i] = indices[i + 1];
        }
        indices[len - 1] = RingBufferIndex(rb, indices[len - 1] + 1);

        search_count++;
    }
    // the last one needs to add 1 -----------
    index->count[index->found - 1]++;
    return RINGBUFFER_NO_ERROR;
}

// ============================================================================
/// move the head to specified position
static RingBufferError_e RingBufferMoveHead(RingBuffer_t* rb, int16_t pos) {
    // cannot move head out of range
    if ((pos < 0) || (pos >= rb->capacity)) {
        return RINGBUFFER_ERR_OOR;
    }

    if (rb->head < rb->tail) {
        if ((pos >= rb->head) && (pos < rb->tail)) {
            rb->count -= (pos - rb->head);
            rb->head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else {
            return RINGBUFFER_ERR_OOR;
        }
    }
    else if (rb->head == rb->tail) {
        return RINGBUFFER_ERR_CNM;
    }
    else if (rb->head > rb->tail) {
        if (pos >= rb->head) {
            rb->count -= (pos - rb->head);
            rb->head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else if (pos < rb->tail) {
            rb->count -= (rb->capacity - rb->head) + pos;
            rb->head = pos;
            return RINGBUFFER_NO_ERROR;
        }
        else {  // out of range
            return RINGBUFFER_ERR_OOR;
        }
    }
    return RINGBUFFER_NO_ERROR;
}
// ============================================================================
static void RingBufferShow(RingBuffer_t* rb, char style, uint16_t width) {
    if (!rb->initialized) {
        rb_printk(0, "%s(): error: not initialized!\r\n", __func__);
        return;
    }
    width = (width < 5) ? 5 : width;  // force to show at least 5 items in a row
    rb_printk(0,
              "--------------\r\nringbuffer (capacity = %d, count = %d, " HGRN
              "head" NOC ", " HCYN "tail" NOC ", " HYLW "head & tail" NOC
              ")\r\n",
              rb->capacity, rb->count);
    for (int16_t i = 0; i < rb->capacity; i++) {
        if ((rb->head == i) && (rb->tail == i)) {
            rb_printk(0, HYLW);
        }
        else if (rb->head == i) {
            rb_printk(0, HGRN);
        }
        else if (rb->tail == i) {
            rb_printk(0, HCYN);
        }
        switch (style) {
        case 'h':
        case 'H':
            rb_printk(0, " %3X ", rb->buffer[i]);
            break;
        case 'd':
        case 'D':
            rb_printk(0, " %3d ", rb->buffer[i]);
            break;
        }
        if ((rb->head == i) || (rb->tail == i)) {
            rb_printk(0, NOC);
        }
        if ((i + 1) % width == 0) {
            rb_printk(0, "\r\n");
        }
    }
    rb_printk(0, "\r\n");
}

// ============================================================================
static void RingBufferInsight(RingBufferIndex_t* index) {
    rb_printk(0, "--------\r\n found %d pattern(s):\r\n", index->found);
    for (int i = 0; i < index->found; i++) {
        rb_printf("%3d (%3d)\r\n", index->pos[i], index->count[i]);
    }
    ( void )index;
}

// ============================================================================
// clang-format off
RingBufferApi_t ringbuffer = {
    .config  = RingBufferConfig  ,
    .push    = RingBufferPush    ,
    .pushN   = RingBufferPushN   ,
    .pop     = RingBufferPop     ,
    .popN    = RingBufferPopN    ,
    .search  = RingBufferSearch  ,
    .move    = RingBufferMoveHead,
    .show    = RingBufferShow    ,
    .insight = RingBufferInsight ,
};
// clang-format on
