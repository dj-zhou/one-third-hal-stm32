#include "config.h"

#include "protocol.h"
#include <math.h>

uint8_t usart2_rx[100];

CommBatteryInfo_t battery_info = {
    .type = CommBatteryInfo,
    .voltage = (float)11.2,
    .current = (float)0.3,
    .soc = (float)0.78,
};

// ============================================================================
void Usart2IdleIrq(void) {
    usart1.ring.show('h', 20);
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
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

    // usart2 is used for communication
    usart2.config(2000000, 8, 'n', 1);
    usart2.dma.config(usart2_rx, sizeof_array(usart2_rx));
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
