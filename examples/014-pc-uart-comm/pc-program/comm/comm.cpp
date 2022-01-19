#include "comm.h"
#include <iostream>

PcComm::PcComm(Serial* serial) {
    if (serial->fd() != -1) {
        serial_ = serial;
    }
    else {
        throw std::runtime_error("PcComm(): serial is not initialized");
    }
}

void PcComm::send(const char* str, size_t len) {
    serial_->send((char*)str, len);
}

void PcComm::send(const CommVelCmd_t cmd) {
    size_t size = sizeof(cmd);
    size_t packet_size = size + 9;
    comm_serialize((const char*)&cmd, size, packet_);
    serial_->send((const char*)packet_, packet_size, 1000);
}

void PcComm::send(const CommSwitchMode_t mode) {
    size_t size = sizeof(mode);
    size_t packet_size = size + 9;
    comm_serialize((const char*)&mode, size, packet_);
    serial_->send((const char*)packet_, packet_size, 1000);
}
