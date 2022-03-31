#include "comm.h"
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

PcComm::PcComm(Serial* serial, uint16_t buffer_size, uint8_t max_header_found,
               OnRecv on_recv) {
    if (serial->fd() != -1) {
        serial_ = serial;
    }
    else {
        throw std::runtime_error("PcComm(): serial is not initialized");
    }
    ring_.init(buffer_size, max_header_found);
    thread_send_ = std::thread(&PcComm::thread_send, this);
    thread_recv_ = std::thread(&PcComm::thread_recv, this, on_recv);

    // set thread priority
    sched_param sch_params;
    sch_params.sched_priority = 10;
    pthread_setschedparam(thread_send_.native_handle(), SCHED_RR, &sch_params);
    sch_params.sched_priority = 15;
    pthread_setschedparam(thread_recv_.native_handle(), SCHED_RR, &sch_params);
}

PcComm::~PcComm() {
    if (thread_send_.joinable()) {
        thread_send_.join();
    }
    if (thread_recv_.joinable()) {
        thread_recv_.join();
    }
}
void PcComm::send(const char* str, size_t size) {
    size_t packet_size = size + 9;
    uint8_t serialized_str[1024];  // FIXME: is 1024 enough?
    comm_serialize(str, size, serialized_str);
    CommSendPacket_t packet;
    packet.data = (uint8_t*)malloc(packet_size);
    packet.size = packet_size;
    if (packet.data == NULL) {
        printf("malloc() failed\n");
        return;
    }
    memcpy(packet.data, serialized_str, packet_size);
    send_queue_.push(packet);
}

void PcComm::thread_send() {
    pthread_setname_np(pthread_self(), "pc-comm-send");
    CommSendPacket_t packet;
    while (!getStop()) {
        if (!send_queue_.empty()) {
            packet = send_queue_.front();
            send_queue_.pop();
            serial_->send((char*)packet.data, packet.size);
            // FIXME: should have some waiting time
            free(packet.data);
        }
        else {
            usleep(500);  // 0.5ms
        }
    }
}

void PcComm::thread_recv(OnRecv on_recv) {
    pthread_setname_np(pthread_self(), "pc-comm-recv");
    uint32_t loop_count = 0;
    uint8_t recv[1024];
    while (!getStop()) {
        ssize_t bytes = read(serial_->fd(), recv, sizeof(recv));
        // if (bytes<0), error, serial cable loose, etc
        if (bytes > 0) {
            ring_.push(recv, (uint16_t)bytes);
        }
        uint8_t header[] = { 0xAB, 0xCD, 0xEF };
        int8_t search_ret = ring_.search(header, 3, 3, 2);
        if (search_ret > 0) {
            uint8_t array[25] = { 0 };
            uint16_t packet_size;
            search_ret = ring_.fetch(array, sizeof(array), &packet_size);
            on_recv((const uint8_t*)array, packet_size);
        }

        // loop control --------------
        usleep(100);  // 0.1ms
        loop_count++;
    }
}

// ============================================================================
void PcComm::setStop() {
    std::lock_guard<std::mutex> lock(thread_mutex_);
    thread_stop_ = true;
}

bool PcComm::getStop() {
    std::lock_guard<std::mutex> lock(thread_mutex_);
    bool stop = thread_stop_;
    return stop;
}

// ============================================================================
void PcComm::send(const CommVelCmd_t cmd) {
    send((const char*)&cmd, sizeof(cmd));
}

void PcComm::send(const CommSwitchMode_t mode) {
    send((const char*)&mode, sizeof(mode));
}
