#include "config.h"
#include "operation.h"
#include "ring-buffer.h"
#include <math.h>

// ============================================================================
static void tfmini_parse(uint8_t* data, uint16_t len) {
    // calculate the checksum ----------
    if (len != 9) {
        console.printf("maybe it is not a tfmini packet?\r\n");
        return;
    }
    uint16_t check_sum = 0;
    for (int i = 0; i < 9 - 1; i++) {
        check_sum += data[i];
    }
    check_sum &= 0xFF;
    // check the checksum ----------
    int8_t error = data[9 - 1] - check_sum;
    if (error != 0) {
        console.printf("checksum error\r\n");
        return;
    }
    uint16_t dist_mm;
    uint16_t strength;
    float temp_c;
    // mode cm:
    // dist_mm = 10 * (data[3] << 8 | data[2]);  // scaled to mm
    // mode mm:
    dist_mm = data[3] << 8 | data[2];
    strength = data[5] << 8 | data[4];
    temp_c = ( float )(data[7] << 8 | data[6]) / 8.0 - 256;
    console.printf("dist = %d mm, ", dist_mm);
    console.printf("strength = %d, ", strength);
    console.printf("temperature = %3.1f C\r\n", temp_c);
}

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
    console.printf("----------\r\n, push three items to the ringbuffer\r\n");
    op.ringbuffer.push(&rb, 32);
    op.ringbuffer.push(&rb, 24);
    op.ringbuffer.push(&rb, 14);
    op.ringbuffer.show(&rb, 'H', 10);

    uint8_t tfmini_data1[] = { 0x59, 0x59, 0xF7, 0x07, 0xB4,
                               0x01, 0xA8, 0x09, 0x16 };
    op.ringbuffer.pushN(&rb, tfmini_data1, sizeof_array(tfmini_data1));
    op.ringbuffer.show(&rb, 'H', 10);
    op.ringbuffer.push(&rb, 24);
    op.ringbuffer.push(&rb, 11);
    uint8_t tfmini_data2[] = { 0x59, 0x59, 0xF4, 0x07, 0xAD,
                               0x01, 0xA8, 0x09, 0x0C };
    op.ringbuffer.pushN(&rb, tfmini_data2, sizeof_array(tfmini_data2));
    op.ringbuffer.pushN(&rb, tfmini_data2, sizeof_array(tfmini_data2));
    op.ringbuffer.show(&rb, 'H', 10);
    uint8_t tfmini_header[] = { 0x59, 0x59 };
    op.ringbuffer.header(&rb, tfmini_header, sizeof_array(tfmini_header));
    if (op.ringbuffer.search(&rb) != RINGBUFFER_NO_ERROR) {
        console.printf("ringbuffer searh error\r\n");
    }
    op.ringbuffer.insight(&rb);
    console.printf("size of RingBuffer_t = %d\r\n", sizeof(RingBuffer_t));
    // attach a process function
    op.ringbuffer.attach(&rb, tfmini_parse);
    // TODO: uses the index information got from search() to process data
    rb.process(tfmini_data2, sizeof_array(tfmini_data2));
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
