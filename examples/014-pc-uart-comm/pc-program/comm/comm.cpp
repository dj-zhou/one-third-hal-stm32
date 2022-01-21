#include "comm.h"
#include <iostream>
#include <pthread.h>
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

    // set thread priority
    sched_param sch_params;
    sch_params.sched_priority = 10;
    pthread_setschedparam(thread_send_.native_handle(), SCHED_RR, &sch_params);
    sch_params.sched_priority = 15;
    pthread_setschedparam(thread_recv_.native_handle(), SCHED_RR, &sch_params);
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
    pthread_setname_np(pthread_self(), "pc-comm-send");
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
    pthread_setname_np(pthread_self(), "pc-comm-recv");
    for (;;) {
        usleep(500);  // 0.5ms
    }
}
