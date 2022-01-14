#include "config.h"
#include "operation.h"
#include <string.h>

uint8_t tfmini_packet[9];

typedef enum TFminiMeasureMode {
    TFMINI_MODE_CM = 1,
    TFMINI_MODE_PIXHAWK = 2,
    TFMINI_MODE_MM = 3,
} TFminiMeasureMode_e;

TFminiMeasureMode_e measure_mode;
uint16_t dist_mm;
uint16_t strength;
float temp_c;

uint8_t tfmini_buffer[50];

// ============================================================================
void tfmini_set_mode(TFminiMeasureMode_e mode) {
    uint8_t cmd[] = { 0x5A, 0x05, 0x05, 0x00, 0x00 };
    measure_mode = mode;
    switch (measure_mode) {
    case TFMINI_MODE_CM:  // 9 bytes output with cm unit
        cmd[3] = 0x01;
        cmd[4] = 0x65;
        break;
    case TFMINI_MODE_PIXHAWK:  // pixhawk mode
        cmd[3] = 0x02;
        cmd[4] = 0x66;
        break;
    case TFMINI_MODE_MM:  // 9 bytes output with mm unit
        cmd[3] = 0x06;
        cmd[4] = 0x6A;
        break;
    default:
        break;
    }
    usart2.send(cmd, sizeof_array(cmd));
}

// //
// ============================================================================
// static void tfmini_parse(uint8_t* data, uint16_t len) {
//     console.printf("%s(): ", __func__);
//     // calculate the checksum ----------
//     if (len < 9) {
//         console.printf("maybe it is not a tfmini packet?\r\n");
//         return;
//     }
//     uint16_t check_sum = 0;
//     for (int i = 0; i < 9 - 1; i++) {
//         check_sum += data[i];
//     }
//     check_sum &= 0xFF;
//     // check the checksum ----------
//     int8_t error = data[9 - 1] - check_sum;
//     if (error != 0) {
//         console.printf("checksum error\r\n");
//         return;
//     }
//     uint16_t dist_mm;
//     uint16_t strength;
//     float temp_c;
//     // mode cm:
//     // dist_mm = 10 * (data[3] << 8 | data[2]);  // scaled to mm
//     // mode mm:
//     dist_mm = data[3] << 8 | data[2];
//     strength = data[5] << 8 | data[4];
//     temp_c = ( float )(data[7] << 8 | data[6]) / 8.0 - 256;
//     console.printf("dist = %d mm, ", dist_mm);
//     console.printf("strength = %d, ", strength);
//     console.printf("temperature = %3.1f C\r\n", temp_c);
// }

// =============================================================================
void taskPrint(void) {
    for (int i = 0; i < sizeof_array(tfmini_buffer); i++) {
        console.printf("%02X ", tfmini_buffer[i]);
    }
    console.printf("\r\n");
    usart2.ring.show('h', 10);
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

    usart2.config(115200, 8, 'n', 1);
    usart2.ring.show('h', 10);
    usart2.ring.config(tfmini_buffer, sizeof_array(tfmini_buffer));
    tfmini_set_mode(TFMINI_MODE_MM);

    // tasks -----------
    stime.scheduler.attach(1000, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
