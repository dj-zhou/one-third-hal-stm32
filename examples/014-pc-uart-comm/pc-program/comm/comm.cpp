#include "comm.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

PcComm::PcComm(Serial* serial) {
    if (serial->fd() != -1) {
        serial_ = serial;
    }
    else {
        throw std::runtime_error("PcComm(): serial is not initialized");
    }
    thread_send_ = std::thread(&PcComm::thread_send, this);
    thread_recv_ = std::thread(&PcComm::thread_recv, this);
}

void PcComm::send(const char* str, size_t len) {
    serial_->send((char*)str, len);
}

void PcComm::send(const CommVelCmd_t cmd) {
    size_t size = sizeof(cmd);
    size_t packet_size = size + 9;
    comm_serialize((const char*)&cmd, size, packet_);
    CommSendPacket_t packet;
    packet.data = (uint8_t*)malloc(packet_size);
    packet.size = packet_size;
    if (packet.data != NULL) {
        memcpy(packet.data, packet_, packet_size);
        send_queue_.push(packet);
    }
    else {
        printf("malloc() failed\n");
    }
    // serial_->send((const char*)packet_, packet_size, 1000);
}

void PcComm::send(const CommSwitchMode_t mode) {
    size_t size = sizeof(mode);
    size_t packet_size = size + 9;
    comm_serialize((const char*)&mode, size, packet_);
    CommSendPacket_t packet;
    packet.data = (uint8_t*)malloc(packet_size);
    packet.size = packet_size;
    if (packet.data != NULL) {
        memcpy(packet.data, packet_, packet_size);
        send_queue_.push(packet);
    }
    else {
        printf("malloc() failed\n");
    }
    // serial_->send((const char*)packet_, packet_size, 1000);
}

void PcComm::thread_send() {
    CommSendPacket_t packet;
    for (;;) {
        if (!send_queue_.empty()) {
            packet = send_queue_.front();
            send_queue_.pop();
            serial_->send((char*)packet.data, packet.size);
        }
        else {
            usleep(500);  // 0.5ms
        }
    }
}

void PcComm::thread_recv() {
    for (;;) {
        usleep(500);  // 0.5ms
    }
}
