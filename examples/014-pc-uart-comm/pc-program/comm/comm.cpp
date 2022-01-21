#include "comm.h"
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

PcComm::PcComm(Serial* serial, uint16_t buffer_size, uint8_t max_header_found) {
    if (serial->fd() != -1) {
        serial_ = serial;
    }
    else {
        throw std::runtime_error("PcComm(): serial is not initialized");
    }
    ring_.init(buffer_size, max_header_found);
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
    uint32_t loop_count = 0;
    uint8_t recv[1024];
    for (;;) {
        ssize_t bytes = read(serial_->fd(), recv, sizeof(recv));
        // if (bytes<0), error, serial cable loose, etc
        if (bytes > 0) {
            ring_.push(recv, (uint16_t)bytes);
        }
        // test
        if (loop_count == 2000) {
            loop_count = 0;
            ring_.show('h', 20);
        }
        usleep(500);  // 0.5ms
        loop_count++;
    }
}
