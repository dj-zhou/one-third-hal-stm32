#include "config.h"
#include "ring-buffer.h"
#include <math.h>

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    uint8_t buffer[50];
    RingBuffer_t rb = ringbuffer.config(buffer, sizeof_array(buffer));
    ringbuffer.show(&rb, 'h', 10);
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
