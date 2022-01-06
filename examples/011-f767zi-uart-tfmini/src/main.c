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
    usart2.transmit(cmd, sizeof_array(cmd));
}

// // ============================================================================
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

// ============================================================================
void Usart2IdleIrqCallback(void) {
    // uint8_t pattern[2];
    // pattern[0] = 0x59;
    // pattern[1] = 0x59;
    // op.ringbuffer.header(&(usart2.rb), pattern, sizeof_array(pattern));
    // uint8_t packets_count = op.ringbuffer.search(&(usart2.rb));

    // while (packets_count > 0) {
    //     uint8_t tfmini_packet[30] = { 0 };
    //     packets_count = op.ringbuffer.fetch(&(usart2.rb), tfmini_packet,
    //                                         sizeof_array(tfmini_packet));
    //     tfmini_parse(tfmini_packet, sizeof_array(tfmini_packet));
    // }
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    console.printf("_RINGBUFFER_HEADER_MAX_LEN = %d\r\n",
                   _RINGBUFFER_HEADER_MAX_LEN);
    led.config(LED_BREATH);
    usart2.config(115200, 8, 'n', 1);
    // uint8_t rx_buffer[27];
    // usart2.ring.config(rx_buffer, sizeof_array(rx_buffer));
    console.printf("\r\n");
    console.printf("ring buffer configured\r\n");
    tfmini_set_mode(TFMINI_MODE_MM);

    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
