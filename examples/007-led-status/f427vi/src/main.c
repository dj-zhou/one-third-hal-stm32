#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint(void) {
    static int32_t loop = 0;
    double data = -sin((double)loop / 180.0 * 3.1415926);
    char* ptr = (char*)&data;
    console.printf("%5d: data = %f, ", loop++, data);
    for (int i = 0; i < 4; i++) {
        console.printf("%2X ", *ptr++);
    }
    console.printf("\r\n");
}

// ============================================================================
int main(void) {
    utils.system.initClock(176, 44, 88);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);
    // tasks -----------
    stime.scheduler.attach(500, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
