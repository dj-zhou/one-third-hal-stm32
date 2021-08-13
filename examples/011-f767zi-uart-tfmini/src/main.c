#include "config.h"
#include "ring-buffer.h"
#include <string.h>

uint8_t rx_buffer[27];
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

// ============================================================================
static void tfmini_parse(uint8_t* data, int8_t* error_flag) {
    // calculate the checksum ----------
    uint16_t check_sum = 0;
    for (int i = 0; i < 9 - 1; i++) {
        check_sum += data[i];
    }
    check_sum &= 0xFF;
    // check the checksum ----------
    int8_t error = data[9 - 1] - check_sum;
    if (error == 0) {
        *error_flag = 0;
    }
    else if (error == 3) {  // this was seen on Linux SoM
        *error_flag = -7;
    }
    else if (error == -3) {  // this was seen on Linux SoM
        *error_flag = -8;
    }
    else {
        *error_flag = -9;
        dist_mm = 0;
        strength = 0;
        temp_c = 0;
        return;
    }
    //  calculate the data ----------
    switch (measure_mode) {
    case TFMINI_MODE_CM:
        dist_mm = 10 * (data[3] << 8 | data[2]);  // scaled to mm
        strength = data[5] << 8 | data[4];
        temp_c = ( float )(data[7] << 8 | data[6]) / 8.0 - 256;
        break;
    case TFMINI_MODE_PIXHAWK:
        // not implemented
        break;
    case TFMINI_MODE_MM:
        dist_mm = data[3] << 8 | data[2];
        strength = data[5] << 8 | data[4];
        temp_c = ( float )(data[7] << 8 | data[6]) / 8.0 - 256;
        break;
    }
    return;
}

// ============================================================================
void Usart2IdleIrqCallback(void) {
    // ringbuffer.show(&(usart2.rb), 'H', 9);
    RingBufferIndex_t index;
    uint8_t pattern[2];
    pattern[0] = 0x59;
    pattern[1] = 0x59;
    ringbuffer.search(&(usart2.rb), pattern, sizeof_array(pattern), &index);
    // ringbuffer.insight(&index);
    int8_t error_flag;
    if ((index.found > 0) && (index.count[index.found - 1] >= 9)) {
        // move head to the last packet, and then pop a packet
        ringbuffer.move(&(usart2.rb), index.pos[index.found - 1]);
        ringbuffer.popN(&(usart2.rb), tfmini_packet,
                        sizeof_array(tfmini_packet));
        tfmini_parse(tfmini_packet, &error_flag);
        console.printf(" %5d, %5d, %4.2f, %d\r\n", dist_mm, strength, temp_c,
                       error_flag);
    }
    else {
        error_flag = -12;
        console.printf(" %5d, %5d, %4.2f, %d\r\n", 0, 0, 0, error_flag);
    }
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
    usart2.ring.config(rx_buffer, sizeof_array(rx_buffer));
    // usart2.dma.config(rx_buffer, sizeof_array(rx_buffer));  // not working!
    tfmini_set_mode(TFMINI_MODE_MM);
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
