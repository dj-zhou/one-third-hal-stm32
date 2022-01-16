#include "serial-comm.h"
#include "comm-protocol.h"
#include <iostream>
#include <stdlib.h>

PcComm::PcComm(Serial* serial) {
    if (serial->fd() != -1) {
        serial_ = serial;
    }
    else {
        throw std::runtime_error("PcComm(): serial is not initialized");
    }
}

void PcComm::send(const char* str, size_t len) {
    serial_->send(( char* )str, len);
}

void PcComm::send(const CommVelCmd_t cmd) {
    size_t size = sizeof(cmd);
    char* str = ( char* )malloc(size + 10);
    comm_serialize(( const char* )&cmd, size, str);
    serial_->send(str, size + 10);
    free(str);
}

void PcComm::send(const CommSwitchMode_t mode) {
    size_t size = sizeof(mode);
    char* str = ( char* )malloc(size + 10);
    comm_serialize(( const char* )&mode, size, str);
    serial_->send(str, size + 10);
    free(str);
}
