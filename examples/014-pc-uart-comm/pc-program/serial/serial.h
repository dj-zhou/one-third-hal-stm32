#pragma once

#include <stdint.h>
#include <stdio.h>

class Serial {
public:
    Serial(const char* dev, int baud_rate, char parity);
    ~Serial();
    int quit();
    bool send(char* str, size_t len);
    bool send(char* str, size_t len, uint32_t exec_time_us);

private:
    int fd_;
};
