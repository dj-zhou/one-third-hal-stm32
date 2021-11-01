
#include "config.h"

// ============================================================================
int main(void) {
    utils.system.initClock(72, 36, 72);
    utils.system.initNvic(4);
    stime.config();
    console.config(2000000);
    console.printf("\r\n");

    uint32_t loop_count = 0;
    Stime_t time = stime.getTime();
    Stime_t time_old = time;

    while (1) {
        time = stime.getTime();
        console.printf("time.s = %5d, ", time.s);
        console.printf("time.us = %6d, ", time.us);
        uint32_t diff =
            (time.s - time_old.s) * 1000000 + (time.us - time_old.us);
        console.printf("diff = %6ld us, diff2 = %3ld us\r\n", diff,
                       diff - 500000);
        stime.delay.us(500000);
        time_old = time;
        loop_count++;
    }
    return 0;
}
