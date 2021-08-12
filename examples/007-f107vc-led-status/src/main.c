
#include "config.h"
// =============================================================================
void taskPrint(void) {
    static int32_t loop = 0;
    console.printf("%5d: hello %s\r\n", loop++, FIRMWARE);
    uint16_t data = 0x5213;
    console.printf("data = %b\r\n", data);
    if (_CHECK_BIT(data, 9)) {
        console.printf("is one\r\n");
    }
    _SET_BIT(data, 11);
    _RESET_BIT(data, 9);
    console.printf("data = %b\r\n", data);
}

// ============================================================================
int main(void) {
    utils.system.initClock(72, 36, 72);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOC, 7, GPIO_MODE_OUTPUT_PP);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_DOUBLE_BLINK);
    // tasks -----------
    stime.scheduler.attach(1000, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system starts to run -----------
    stime.scheduler.run();

    // it would never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
