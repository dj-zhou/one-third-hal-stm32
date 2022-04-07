#include "comm.h"
#include "protocol.h"
#include <stdint.h>

static uint8_t packet_[1024];

void send_packet(void* msg, size_t msg_size) {
    uint16_t packet_size = (uint16_t)(msg_size + 9);
    protocol_serialize((const char*)msg, msg_size, packet_);
    // FIXME: it can be usart1
    usart2.send(packet_, packet_size);
}
