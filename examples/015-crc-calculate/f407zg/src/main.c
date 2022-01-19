#include "config.h"

#include "crc.h"
#include <math.h>

// ============================================================================
int main(void) {
    utils.pin.mode(GPIOE, 11, GPIO_MODE_OUTPUT_PP);
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    uint8_t data[100];
    for (uint8_t i = 0; i < sizeof_array(data); i++) {
        data[i] = (uint8_t)(i * 3);
    }
    crc.hard.config();
    Stime_t time1 = stime.getTime();
    uint32_t crc_hard_calc = crc.hard._32bit8(data, sizeof_array(data));
    Stime_t time2 = stime.getTime();
    uint32_t crc_soft_calc = crc.soft._32bit8(data, sizeof_array(data));
    Stime_t time3 = stime.getTime();

    console.printf("crc_hard_calc  = 0x%04X\r\n", crc_hard_calc);
    console.printf("crc_soft_calc  = 0x%04X\r\n", crc_soft_calc);

    uint32_t diff1 = (time2.s - time1.s) * 1000000 + (time2.us - time1.us);
    uint32_t diff2 = (time3.s - time2.s) * 1000000 + (time3.us - time2.us);
    console.printf(
        "hardware CRC takes %d us, and software CRC takes %d us.\r\n", diff1,
        diff2);

    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
