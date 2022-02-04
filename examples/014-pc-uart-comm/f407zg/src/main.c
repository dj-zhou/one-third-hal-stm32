#include "config.h"

#include "comm.h"
#include "protocol.h"
#include <math.h>
#include <stdint.h>

// ----------------------------------------------------------------------------
// clang-format off

#ifndef _USART1_MESSAGE_NODE_MAX_NUM
#define _USART1_MESSAGE_NODE_MAX_NUM 50
#endif
#ifndef _USART1_MESSAGE_DESCR_SIZE
#define _USART1_MESSAGE_DESCR_SIZE 50
#endif



typedef void (*usart1_irq_hook)(uint8_t*msg);

// clang-format off
typedef struct Usart1MsgCpnt_s {
    uint16_t     msg_type;
    char         descr[_USART1_MESSAGE_DESCR_SIZE];
    usart1_irq_hook hook;
} Usart1MsgCpnt_t;

typedef struct Usart1MsgNode_s {
    struct Usart1MsgCpnt_s  this_;
    struct Usart1MsgNode_s* next_;
} Usart1MsgNode_t;
// clang-format on

static Usart1MsgNode_t usart1_node[_USART1_MESSAGE_NODE_MAX_NUM] = { 0 };
static uint8_t usart1_node_num = 0;

static bool usart1_msg_attach(uint16_t type, usart1_irq_hook hook) {
    // you cannot attach too many callbacks
    if (usart1_node_num >= _USART1_MESSAGE_NODE_MAX_NUM) {
        console.error("usart1_msg_attach: too many messages attached!\r\n");
    }
    // you cannot attach two callback functions to one message type (on one
    // port)
    if (usart1_node_num > 0) {
        for (uint8_t i = 0; i < usart1_node_num; i++) {
            if (usart1_node[i].this_.msg_type == type) {
                return false;
            }
        }
    }
    if (usart1_node_num == 0) {
        usart1_node[0].this_.msg_type = type;
        usart1_node[0].this_.hook = hook;
        usart1_node[0].next_ = NULL;
        usart1_node_num++;
        return true;
    }

    usart1_node[usart1_node_num].this_.msg_type = type;
    usart1_node[usart1_node_num].this_.hook = hook;
    usart1_node[usart1_node_num - 1].next_ = &usart1_node[usart1_node_num];
    usart1_node_num++;

    return true;
}

static void VelCmdCallback(uint8_t* msg) {
    uint8_t* ptr_msg = msg + 5;
    uint16_t type;
    uint8_t* ptr_type = (uint8_t*)&type;
    *ptr_type++ = *ptr_msg++;
    *ptr_type = *ptr_msg;
    console.printf("VelCmdCallback: type = 0x%04X\r\n", type);
}

static void SwitchModeCallback(uint8_t* msg) {
    uint8_t* ptr_msg = msg + 5;
    uint16_t type;
    uint8_t* ptr_type = (uint8_t*)&type;
    *ptr_type++ = *ptr_msg++;
    *ptr_type = *ptr_msg;
    console.printf("SwitchModeCallback: type = 0x%04X\r\n", type);
}

// ============================================================================
uint8_t usart1_rx[100];

CommBatteryInfo_t battery_info = {
    .type = CommBatteryInfo,
    .voltage = (float)11.2,
    .current = (float)0.3,
    .soc = (float)0.78,
};

CommVelCmd_t vel_cmd = {
    .type = CommVelCmd,
    .x_vel = (float)0.8,
    .y_vel = (float)0.4,
    .yaw_vel = (float)0.2,
};

// ============================================================================
void Usart1IdleIrq(void) {
    // usart1.ring.show('h', 10);

    int8_t search_ret = usart1.ring.search();
    if (search_ret > 0) {
        // op.ringbuffer.insight(&usart1.rb);
        console.printf("find %d packets\r\n", search_ret);
        uint8_t array[25] = { 0 };
        search_ret = usart1.ring.fetch(array, sizeof_array(array));
        // for (int i = 0; i < 25; i++) {
        //     console.printf("%02X  ", array[i]);
        // }
        // console.printf("\r\nafter fetch:\r\n");
        // usart1.ring.show('h', 10);

        uint8_t* ptr_msg = array + 5;
        uint16_t type;
        uint8_t* ptr_type = (uint8_t*)&type;
        *ptr_type++ = *ptr_msg++;
        *ptr_type = *ptr_msg;

        for (uint8_t i = 0; i < usart1_node_num; i++) {
            if (usart1_node[i].this_.msg_type == type) {
                usart1_node[i].this_.hook(array);
                return;
            }
        }
    }
    if (search_ret < 0) {
        op.ringbuffer.error(search_ret);
    }
}

// ============================================================================
void taskSend(void) {
    static uint32_t loop_count = 0;
    // ----------------
    battery_info.voltage = (float)(10.5 + sin(loop_count / 100.));
    send_packet((void*)&battery_info, sizeof(battery_info));
    stime.delay.ms(100);
    // (test only) ----------------
    send_packet((void*)&vel_cmd, sizeof(vel_cmd));
    loop_count++;
}

// ============================================================================
int main(void) {
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n\r\n");
    led.config(LED_BREATH);

    console.printf("sizeof(CommVelCmd_t) = %ld\r\n", sizeof(CommVelCmd_t));
    console.printf("sizeof(CommSwitchMode_t) = %ld\r\n",
                   sizeof(CommSwitchMode_t));
    console.printf("sizeof(CommPoseInfo_t) = %ld\r\n", sizeof(CommPoseInfo_t));
    console.printf("sizeof(CommBatteryInfo_t) = %ld\r\n",
                   sizeof(CommBatteryInfo_t));
    console.printf("sizeof(CommFirmwareInfo_t) = %ld\r\n",
                   sizeof(CommFirmwareInfo_t));

    // usart1 is used for communication
    usart1.config(2000000, 8, 'n', 1);
    usart1.dma.config(usart1_rx, sizeof_array(usart1_rx));
    uint8_t header[] = { 0xAB, 0xCD, 0xEF };
    usart1.ring.set.header(header, sizeof_array(header));
    usart1.ring.set.length(3, 2);
    usart1_msg_attach(CommVelCmd, VelCmdCallback);
    usart1_msg_attach(CommSwitchMode, SwitchModeCallback);

    // tasks -----------
    stime.scheduler.attach(1000, 200, taskSend, "taskSend");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
