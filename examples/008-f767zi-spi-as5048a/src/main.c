#include "config.h"
#include <math.h>

// AS5048A commands -----------------------
// to verify
#define CMD_ANGLE 0xFFFF    // read (bit 14: 1)
#define CMD_AGC 0x7FFD      // read
#define CMD_MAG 0x7FFE      // read
#define CMD_CLAER 0x4001    // write (bit 14: 0)
#define CMD_NOP 0xC000      // ...
#define CMD_ZERO_HI 0x8016  // write
#define CMD_ZERO_LO 0x0017  // write

// =============================================================================
static void taskPrint(void) {
    static int32_t  loop       = 220;
    float           data       = -sin(( double )loop / 180.0 * 3.1415926);
    char*           ptr        = ( char* )&data;
    static uint32_t loop_count = 0;
    console.printf("-------------------\r\n");
    console.printf("%5d,data = %f, ", loop_count++, data);
    for (int i = 0; i < 4; i++) {
        console.printf("%X ", *ptr++);
    }
    console.printf("\r\n");
    uint32_t addr1 = ( uint32_t )&spi1;
    uint32_t addr2 = ( uint32_t )&spi1.transceive8bits;
    console.printf("address of spi1 = %08d\r\n", addr1);
    console.printf("address of spi1.transceive8bits = %08d\r\n", addr2);
    console.printf("address diff = %d\r\n", addr2 - addr1);
    SpiApi_t* spi =
        container_of(&(spi1.transceive8bits), SpiApi_t, transceive8bits);
    console.printf("address of spi = %08d\r\n", ( uint32_t )spi);
}

// ============================================================================
static void readAM4096(void) {
    static uint32_t loop_count = 0;
    uint16_t        no_op      = 0x0000;
    uint16_t        raw;
    spi1.transceive16bits(&no_op, &raw, 1);
    console.printf("%5d: raw = %b = %5d, trimmed: %d\r\n", loop_count++, raw,
                   raw, (raw & 0x7FFF) >> 3);
}

// ============================================================================
static void resetEncoder(void) {
    bool status = HAL_GPIO_ReadPin(GPIOC, 1 << 13);
    if (status == true) {
        utils.pin.set(GPIOB, 4, true);
    }
    else {
        utils.pin.set(GPIOB, 4, false);
    }
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000, 8, 'n', 1);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);
    spi1.config(32, "master", "soft", "high", "falling");
    spi1.setNss(GPIOF, 12);
    utils.pin.mode(GPIOB, 4, GPIO_MODE_OUTPUT_PP);  // reset, pull it to HIGH

    utils.pin.mode(GPIOC, 13, GPIO_MODE_INPUT);  // reset, pull it to HIGH
    utils.pin.pull(GPIOC, 13, GPIO_PULLDOWN);
    // tasks -----------
    // stime.scheduler.attach( 500, 2, taskPrint, "taskPrint" );
    ( void )taskPrint;
    stime.scheduler.attach(100, 2, readAM4096, "readAM4096");
    stime.scheduler.attach(5, 1, resetEncoder, "resetEncoder");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
