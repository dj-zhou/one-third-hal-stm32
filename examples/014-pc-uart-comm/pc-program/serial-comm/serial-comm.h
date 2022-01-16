#pragma once

#include "comm-protocol.h"
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
};
