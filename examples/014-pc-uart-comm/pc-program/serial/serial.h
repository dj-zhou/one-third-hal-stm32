#pragma once

#include <stdint.h>
#include <stdio.h>

class Serial {
public:
    Serial(const char* dev, int baud_rate, char parity);
    ~Serial();
    int quit();
    bool send(const char* str, size_t len);
    bool send(const char* str, size_t len, uint32_t exec_time_us);
    int fd();

private:
    int fd_;

    Serial(const Serial&) = delete;             // non construction-copyable
    Serial& operator=(const Serial&) = delete;  // non copyable
};
