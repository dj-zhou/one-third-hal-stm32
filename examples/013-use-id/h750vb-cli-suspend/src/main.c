#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint(void) {
    static int32_t loop = 0;
    double data = sin((double)loop / 180.0 * 3.1415926);
    char* ptr = (char*)&data;
    console.printf("%5d, %s | data = %f, ", loop++, FIRMWARE, data);
    for (int i = 0; i < 4; i++) {
        console.printf(" %02X", *ptr++);
    }
    console.printf(", FPU type = %d\r\n", SCB_GetFPUType());
}

// ============================================================================
int main(void) {
    utils.system.initClock(480, 120, 120);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_DOUBLE_BLINK);

    // tasks -----------
    stime.scheduler.attach(500, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
