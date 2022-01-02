#include "config.h"
#include "operation.h"
#include "ring-buffer.h"
#include <math.h>

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\nmain() starts here\r\n");
    console.printf("make time: %s, %s\r\n", __TIME__, __DATE__);
    led.config(LED_BREATH);

    uint8_t buffer[30];
    RingBuffer_t rb = op.ringbuffer.init(buffer, sizeof_array(buffer));

    // show the ringbuffer after it is initizized
    op.ringbuffer.show(&rb, 'H', 10);
    op.ringbuffer.show(&rb, 'd', 10);
    // push data into the ringbuffer
    op.ringbuffer.push(&rb, 0x45);
    op.ringbuffer.show(&rb, 'H', 10);
    uint8_t data[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    op.ringbuffer.pushN(&rb, data, 10);
    // fully fill the ringbuffer
    op.ringbuffer.pushN(&rb, data, 9);
    op.ringbuffer.pushN(&rb, data, 10);
    op.ringbuffer.show(&rb, 'H', 10);
    op.ringbuffer.pushN(&rb, data, 6);
    op.ringbuffer.show(&rb, 'H', 10);

    // reset the buffer
    op.ringbuffer.reset(&rb);
    op.ringbuffer.show(&rb, 'H', 10);
    op.ringbuffer.pushN(&rb, data, 10);
    op.ringbuffer.show(&rb, 'H', 10);

    // pop one byte data outfrom the ringbuffer
    uint8_t ret;
    op.ringbuffer.pop(&rb, &ret);
    console.printf("pop 1 byte, ret = %d\r\n", ret);
    op.ringbuffer.show(&rb, 'H', 10);
    // pop data when the buffer has no content (count = 0)
    op.ringbuffer.reset(&rb);
    op.ringbuffer.push(&rb, 45);
    op.ringbuffer.show(&rb, 'H', 10);
    if (op.ringbuffer.pop(&rb, &ret)) {
        console.printf("pop() succeed, ret = %d\r\n", ret);
    }
    else {
        console.printf("pop() failed\r\n");
    }

    // tfmini data packet
    op.ringbuffer.push(&rb, 32);
    op.ringbuffer.push(&rb, 24);
    op.ringbuffer.push(&rb, 14);
    uint8_t tfmini_data[] = { 0x59, 0x59, 0xF7, 0x07, 0xB4,
                              0x01, 0xA8, 0x09, 0x16 };

    console.printf("sizeof_array(tfmini_data) = %d\r\n",
                   sizeof_array(tfmini_data));
    op.ringbuffer.show(&rb, 'H', 10);
    op.ringbuffer.pushN(&rb, tfmini_data, sizeof_array(tfmini_data));
    op.ringbuffer.show(&rb, 'H', 10);
    uint8_t tfmini_header[] = { 0x59, 0x59 };
    op.ringbuffer.header(&rb, tfmini_header, sizeof_array(tfmini_header));

    console.printf("size = %d\r\n", sizeof(RingBuffer_t));
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
