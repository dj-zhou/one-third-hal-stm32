#pragma once

#include "protocol.h"
#include "serial.h"
#include <stdint.h>

class PcComm {
public:
    PcComm(Serial* serial);
    ~PcComm(){};
    void send(const char* str, size_t len);
    void send(const CommVelCmd_t cmd);
    void send(const CommSwitchMode_t mode);

private:
    Serial* serial_;
    uint8_t packet_[1024];  // FIXME: is 1024 enough?
};
