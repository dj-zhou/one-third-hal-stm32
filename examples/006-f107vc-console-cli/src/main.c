
#include "config.h"
// =============================================================================
void taskHeartBeat(void) {
    utils.pin.toggle(GPIOD, 4);
}

// ============================================================================
int main(void) {
    utils.system.initClock(72, 36, 72);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOD, 4, GPIO_MODE_OUTPUT_PP);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    stime.scheduler.attach(200, 2, taskHeartBeat, "taskHeartBeat");
    // also try _1_TICK, _2_TICK and _3_TICK
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
