#include "comm-protocol.h"

#define COMM_HEADER_1 0xAB
#define COMM_HEADER_2 0xCD
#define COMM_HEADER_3 0xEF

#define COMM_TAIL_1 0xFE
#define COMM_TAIL_2 0xDC
#define COMM_TAIL_3 0xBA

void comm_serialize(const char* data, size_t data_size, char* packet) {
    size_t len = data_size + 5;
    uint8_t* ptr = ( uint8_t* )packet;
    // header
    *ptr++ = COMM_HEADER_1;
    *ptr++ = COMM_HEADER_2;
    *ptr++ = COMM_HEADER_3;
    // length
    *ptr++ = ( uint8_t )(len & 0x00FF);
    *ptr++ = ( uint8_t )((len & 0xFF00) >> 8);
    // payload
    uint8_t* ptr_data = ( uint8_t* )data;
    for (size_t i = 0; i < data_size; i++) {
        *ptr++ = *ptr_data++;
    }
    // TODO: crc16
    uint16_t crc16 = 0xAABB;
    *ptr++ = ( uint8_t )(crc16 & 0x00FF);
    *ptr++ = ( uint8_t )((crc16 & 0xFF00) >> 8);
    // tail
    *ptr++ = COMM_TAIL_1;
    *ptr++ = COMM_TAIL_2;
    *ptr = COMM_TAIL_3;
}
