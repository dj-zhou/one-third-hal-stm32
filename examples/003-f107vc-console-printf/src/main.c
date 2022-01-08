#include "config.h"

// ============================================================================
int main(void) {
    utils.system.initClock(72, 36, 72);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOD, 4, GPIO_MODE_OUTPUT_PP);
    stime.config();
    console.config(2000000);
    int loop_count = 0;

    while (1) {
        // just some delay
        for (int i = 0; i < 3000; i++) {
            for (int j = 0; j < 3000; j++) {
                ;
            }
        }
        utils.pin.toggle(GPIOD, 4);

        console.printf(
            "\r\n\r\n---------------------------------------------\r\n");
        console.printf(YLW "%s\r\n " NOC, FIRMWARE);
        console.printf("%5d = %b\r\n", loop_count, loop_count);
        loop_count++;
        double pi = 3.1415926;
        console.printf(GRN "pi = %f\r\n" NOC, pi);
        const char str[] = "this is a one-third demo program";
        console.printf(" %s\r\n", str);
        int data_int = 3752;
        console.printf(" data_int = %d, %o, %0X\r\n", data_int, data_int,
                       data_int);
        console.printf(" data_int = %0x, %ld, %lu\r\n", data_int, data_int,
                       data_int);
    }
}
