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
} CommSendPacket_t;

class PcComm {
public:
    PcComm(Serial* serial, uint16_t buffer_size, uint8_t max_header_found);
    ~PcComm();
    void send(const char* str, size_t len);
    void setStop();
    bool getStop();
    // seems not necessary
    void send(const CommVelCmd_t cmd);
    void send(const CommSwitchMode_t mode);

private:
    Serial* serial_;
    RingBuffer ring_;
    std::queue<CommSendPacket_t> send_queue_;
    std::thread thread_send_;
    std::thread thread_recv_;
    std::mutex thread_mutex_;
    bool thread_stop_;

protected:
    void thread_send();
    void thread_recv();
};
