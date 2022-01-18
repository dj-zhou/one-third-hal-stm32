#include "config.h"
#include <math.h>

#pragma pack(1)
typedef struct Parameters_s {
    uint16_t key;
    uint8_t a;
    uint16_t b[4];
    float c;
    double d;
} Parameters_t;
#pragma pack()

Parameters_t param1;
Parameters_t param2;

static void initParameters(void) {
    param1.key = EEPROM_KEY_01;
    param1.a = 1;
    param1.b[0] = 10;
    param1.b[1] = 20;
    param1.b[2] = 30;
    param1.b[3] = 40;
    param1.c = -987;
    param1.d = 152;

    param2.key = EEPROM_KEY_02;
    param2.a = 16;
    param2.b[0] = 1111;
    param2.b[1] = 2222;
    param2.b[2] = 4444;
    param2.b[3] = 5555;
    param2.c = ( float )111.24;
    param2.d = -5555.32;
}

static void printParameter(Parameters_t param) {
    console.printf("parameter data:\r\n");
    console.printf("   key = 0x%02X\r\n", param.key);
    console.printf("   a = %d\r\n", param.a);
    console.printf("   b = %d, %d, %d, %d\r\n", param.b[0], param.b[1],
                   param.b[2], param.b[3]);
    console.printf("   c = %3.1f\r\n", param.c);
    console.printf("   d = %3.1f\r\n", param.d);
}

// ============================================================================
int main(void) {
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);
    iic1.config(400000);
    eeprom.config();

    uint8_t data_to_write[1024];
    uint8_t data_to_read[1024];
    uint8_t data_to_compare[1024];
    for (uint16_t i = 0; i < 1024; i++) {
        data_to_write[i] = (uint8_t)(i + 10);
    }
    uint16_t total_bytes_to_write = 20;

    // test 1: write to eeprom byte by byte --------------
    console.printf("\r\n\r\n===============================\r\n");
    console.printf("write to eeprom byte by byte of %d bytes, need %d ms to "
                   "wait:\r\n",
                   total_bytes_to_write, total_bytes_to_write * 10);
    stime.tic();
    for (uint16_t i = 0; i < total_bytes_to_write; i++) {
        eeprom.write.byte(i, data_to_write[i]);
    }
    stime.toc("ms", "writing to eeprom");
    for (uint16_t i = 0; i < total_bytes_to_write; i++) {
        data_to_read[i] = eeprom.read.byte(i);
    }
    uint32_t sum = 0;
    for (uint16_t i = 0; i < total_bytes_to_write; i++) {
        data_to_compare[i] = (uint8_t)(data_to_read[i] - data_to_write[i]);
        sum += data_to_compare[i];
    }
    console.printf("compare sum = %d\r\n", sum);
    if (sum == 0) {
        console.printf("writing and reading are successful!\r\n");
    }

    // test 2: write to eeprom at once --------------

    console.printf("\r\n\r\n===============================\r\n");
    console.printf("write to eeprom %d bytes, no need to wait:\r\n",
                   total_bytes_to_write);
    for (uint16_t i = 0; i < 1024; i++) {
        data_to_write[i] = (uint8_t)(i + 15);
    }
    eeprom.write.bytes(0, data_to_write, total_bytes_to_write);
    eeprom.read.bytes(0, data_to_read, total_bytes_to_write);
    sum = 0;
    for (uint16_t i = 0; i < total_bytes_to_write; i++) {
        data_to_compare[i] = (uint8_t)(data_to_read[i] - data_to_write[i]);
        sum += data_to_compare[i];
    }
    console.printf("compare sum = %d\r\n", sum);
    if (sum == 0) {
        console.printf("writing and reading are successful!\r\n");
    }

    // eeprom node test ---------------
    console.printf("\r\n\r\n===============================\r\n");
    console.printf("sizeof(Parameters_t) = %d\r\n", sizeof(Parameters_t));
    initParameters();
    eeprom.node.attach(( uint8_t* )(&param1), sizeof(param1));
    eeprom.node.attach(( uint8_t* )(&param2), sizeof(param2));
    eeprom.node.show();
    eeprom.node.write(( uint8_t* )(&param1), sizeof(param1));
    eeprom.node.write(( uint8_t* )(&param2), sizeof(param2));
    Parameters_t param_read;
    param_read.key = param1.key;  // revise this to see what happens
    eeprom.node.read(( uint8_t* )(&param_read), sizeof(param_read));
    printParameter(param_read);

    // tasks -----------
    stime.scheduler.show();
    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
