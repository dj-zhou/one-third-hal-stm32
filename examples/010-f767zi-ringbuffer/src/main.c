#include "config.h"
#include "ring-buffer.h"
#include <string.h>

uint8_t rx_buffer[27];

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    RingBufferIndex_t index;
    RingBuffer_t      rb = ringbuffer.config(rx_buffer, 27);
    console.printf("address of rx_buffer: 0x%08X\r\n", rx_buffer);
    console.printf("          rb.buffer : 0x%08X\r\n", rb.buffer);
    console.printf("            rb.head : 0x%08X\r\n", rb.head);
    console.printf("            rb.tail : 0x%08X\r\n", rb.tail);
    console.printf("        rb.capacity : %d\r\n", rb.capacity);
    console.printf("           rb.count : %d\r\n", rb.count);
    ringbuffer.show(&rb, 'd', 9);
    ringbuffer.push(&rb, 56);
    ringbuffer.show(&rb, 'd', 9);
    uint8_t data[] = { 11, 59, 59, 44, 55, 66, 59, 59, 99 };
    ringbuffer.pushN(&rb, data, 9);
    ringbuffer.show(&rb, 'd', 9);
    uint8_t hello[100];
    ( void )hello;
    uint8_t ret[8];
    if (ringbuffer.popN(&rb, ret, 8)) {
        console.printf("pop 8 items\r\n");
        ringbuffer.show(&rb, 'd', 9);
    }

    for (int i = 0; i < 8; i++) {
        console.printf(" %d ", ret[i]);
    }
    console.printf("\r\n");
    ringbuffer.pushN(&rb, data, 9);
    ringbuffer.show(&rb, 'd', 9);
    uint8_t pattern[2];
    pattern[0] = 59;
    pattern[1] = 59;
    ringbuffer.search(&rb, pattern, sizeof_array(pattern), &index);
    for (int i = 0; i < index.found; i++) {
        console.printf("found pattern at position: %d\r\n", index.pos[i]);
    }
    ringbuffer.push(&rb, 59);
    ringbuffer.push(&rb, 59);
    ringbuffer.show(&rb, 'd', 10);
    ringbuffer.search(&rb, pattern, sizeof_array(pattern), &index);
    for (int i = 0; i < index.found; i++) {
        console.printf("index.pos[%d]: %d, count[%d]: %d\r\n", i, index.pos[i],
                       i, index.count[i]);
    }
    ringbuffer.push(&rb, 15);
    ringbuffer.push(&rb, 14);
    uint8_t data1;
    ringbuffer.pop(&rb, &data1);
    console.printf("data1 = %d\r\n", data1);
    ringbuffer.show(&rb, 'd', 10);
    ringbuffer.search(&rb, pattern, sizeof_array(pattern), &index);
    for (int i = 0; i < index.found; i++) {
        console.printf("index.pos[%d]: %d, count[%d]: %d\r\n", i, index.pos[i],
                       i, index.count[i]);
    }

    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
