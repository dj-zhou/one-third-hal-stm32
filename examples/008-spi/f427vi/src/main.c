#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint(void) {
    static int32_t loop = 220;
    double data = -sin((double)loop / 180.0 * 3.1415926);
    char* ptr = (char*)&data;
    static uint32_t loop_count = 0;
    console.printf("-------------------\r\n");
    console.printf("%5d,data = %f, ", loop_count++, data);
    for (int i = 0; i < 4; i++) {
        console.printf("%X ", *ptr++);
    }
    console.printf("\r\n");
    uint32_t addr1 = (uint32_t)&spi1;
    uint32_t addr2 = (uint32_t)&spi1.transceive8bits;
    console.printf("address of spi1 = %08d\r\n", addr1);
    console.printf("address of spi1.transceive8bits = %08d\r\n", addr2);
    console.printf("address diff = %d\r\n", addr2 - addr1);
    SpiApi_t* spi =
        container_of(&(spi1.transceive8bits), SpiApi_t, transceive8bits);
    console.printf("address of spi = %08d\r\n", (uint32_t)spi);
}

// ============================================================================
int main(void) {
    utils.pin.mode(GPIOE, 11, GPIO_MODE_OUTPUT_PP);
    utils.system.initClock(180, 45, 90);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_DOUBLE_BLINK);
    spi1.config(16, "master", "soft", "high", "falling");
    spi1.setNss(GPIOC, 2);
    // tasks -----------
    stime.scheduler.attach(500, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
