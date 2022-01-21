#pragma once

#include "protocol.h"
#include "ringbuffer.h"
#include "serial.h"
#include <queue>
#include <stdint.h>
#include <thread>

typedef struct {
    uint8_t* data;
    size_t size;
    // bool _free;
} CommSendPacket_t;

class PcComm {
public:
    PcComm(Serial* serial, uint16_t buffer_size, uint8_t max_header_found);
    ~PcComm(){};
    void send(const char* str, size_t len);
    void send(const CommVelCmd_t cmd);
    void send(const CommSwitchMode_t mode);

private:
    Serial* serial_;
    uint8_t packet_[1024];  // FIXME: is 1024 enough?
    RingBuffer ring_;
    std::queue<CommSendPacket_t> send_queue_;
    std::thread thread_send_;
    std::thread thread_recv_;

protected:
    void thread_send();
    void thread_recv();
};
