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
void taskPrint(void) {
    static int32_t loop = 220;
    float data = -sin(( double )loop / 180.0 * 3.1415926);
    char* ptr = ( char* )&data;
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
void readAS5048A(void) {
    static uint32_t loop_count = 0;
    uint16_t cmd_read = CMD_ANGLE;
    uint16_t angle_read;
    uint16_t cmd_reset;
    spi1.transceive16bits(&cmd_read, &angle_read, 1);
    if (loop_count == 50) {
        console.printf("try to reset\r\n\r\n");
        cmd_reset = CMD_ZERO_HI;
        spi1.transceive16bits(&cmd_reset, &angle_read, 1);
        stime.delay.us(100);
        cmd_reset = CMD_ZERO_LO;
        spi1.transceive16bits(&cmd_reset, &angle_read, 1);
    }
    _SWAP_16(angle_read);
    console.printf("%5d: angle = %6d\r\n", loop_count++, angle_read & 0x3fff);
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);
    spi1.config(16, "master", "soft", "high", "falling");
    spi1.setNss(GPIOF, 12);

    // tasks -----------
    // stime.scheduler.attach( 500, 2, taskPrint, "taskPrint" );
    ( void )taskPrint;
    stime.scheduler.attach(100, 2, readAS5048A, "readAS5048A");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
