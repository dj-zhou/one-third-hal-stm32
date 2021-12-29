#include "serial.h"
#include <chrono>
#include <fcntl.h>
#include <fmt/core.h>
#include <iostream>
#include <termios.h>
#include <thread>
#include <unistd.h>

using control_clock = std::chrono::steady_clock;

static int get_baud_rate(int baud) {
    switch (baud) {
    case 50:
    case B50:
        return B50;
    case 75:
    case B75:
        return B75;
    case 110:
    case B110:
        return B110;
    case 134:
    case B134:
        return B134;
    case 150:
    case B150:
        return B150;
    case 200:
    case B200:
        return B200;
    case 300:
    case B300:
        return B300;
    case 600:
    case B600:
        return B600;
    case 1200:
    case B1200:
        return B1200;
    case 1800:
    case B1800:
        return B1800;
    case 2400:
    case B2400:
        return B2400;
    case 4800:
    case B4800:
        return B4800;
    case 9600:
    case B9600:
        return B9600;
    case 19200:
    case B19200:
        return B19200;
    case 38400:
    case B38400:
        return B38400;
    case 57600:
    case B57600:
        return B57600;
    case 115200:
    case B115200:
        return B115200;
    case 230400:
    case B230400:
        return B230400;
    case 460800:
    case B460800:
        return B460800;
    case 500000:
    case B500000:
        return B500000;
    case 576000:
    case B576000:
        return B576000;
    case 921600:
    case B921600:
        return B921600;
    case 1000000:
    case B1000000:
        return B1000000;
    case 1152000:
    case B1152000:
        return B1152000;
    case 1500000:
    case B1500000:
        return B1500000;
    case 2000000:
    case B2000000:
        return B2000000;
    case 2500000:
    case B2500000:
        return B2500000;
    case 3000000:
    case B3000000:
        return B3000000;
    case 3500000:
    case B3500000:
        return B3500000;
    case 4000000:
    case B4000000:
        return B4000000;
    }
    return B0;
}

Serial::Serial(const char* dev, int baud_rate, char parity) {
    fd_ = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC | O_CLOEXEC);
    if (-1 == fd_) {
        throw std::runtime_error(fmt::format("Cannot open device: {}", dev));
    }
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    int baud = get_baud_rate(baud_rate);
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);
    // always set 8-bit length
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    // ignore modem controls, enable reading
    tty.c_cflag |= (CLOCAL | CREAD);

    switch (parity) {
    // even
    case 'E':
    case 'e':
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
        break;
    // odd
    case 'O':
    case 'o':
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
        break;
    // none
    case 'N':
    case 'n':
    default:  // 8N1 default config
        tty.c_cflag &= ~PARENB;
        break;
    }

    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~IGNBRK;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~(ONLCR | OCRNL);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    // non-blocking
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        throw std::runtime_error(fmt::format("Error {} ({}) from tcsetattr()",
                                             errno, strerror(errno)));
    }
}

Serial::~Serial() {
    fd_ = -1;
}

int Serial::quit() {
    int ret = close(fd_);
    if (-1 == ret) {
        throw std::runtime_error(fmt::format("close() returns {}", ret));
    }
    return ret;
}

bool Serial::send(char* str, size_t len) {
    size_t offs = 0;
    while (offs < len) {
        ssize_t r = write(fd_, str + offs, len - offs);
        if (r < 0) {
            return false;
        }
        offs += r;
    }
    return false;
}

bool Serial::send(char* str, size_t len, uint32_t exec_time_us) {
    bool ret = send(str, len);
    control_clock::time_point time_ready =
        control_clock::now() + std::chrono::microseconds(exec_time_us);
    std::this_thread::sleep_until(time_ready);
    return ret;
}
