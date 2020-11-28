#include "config.h"
#include <math.h>

// ============================================================================
static void taskSpiAS5048aTest(void) {
    static uint32_t loop_count = 0;
    uint8_t         data_t[3]  = { 11, 22, 33 };
    uint8_t         data_r[3];
    spi1.transceive(data_t, data_r, 3);
    console.printf("%5d: ", loop_count++);
    for (int i = 0; i < 3; i++) {
        console.printf("%d ", data_t[i]);
    }
    console.printf(" | ");
    for (int i = 0; i < 3; i++) {
        console.printf("%d ", data_r[i]);
    }
    console.printf("\r\n");
}

// ============================================================================
int main(void) {
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000, 8, 'n', 1);
    console.printf("\r\n\r\n");
    led.config(LED_DOUBLE_BLINK);
    spi1.config(256, SPI_MASTER, SPI_HARD_NSS);
    // spi1.setNss(GPIOA, 4);
    // tasks -----------
    stime.scheduler.attach(100, 2, taskSpiAS5048aTest, "SpiAS5048aTest");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
