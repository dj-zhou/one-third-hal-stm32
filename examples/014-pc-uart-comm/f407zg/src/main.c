#include "config.h"
#include <math.h>

uint8_t usart1_rx[100];

// ============================================================================
void Usart1IdleIrq(void) {
    usart1.ring.show('h', 10);
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
    usart1.config(2000000, 8, 'n', 1);
    usart1.dma.config(usart1_rx, sizeof_array(usart1_rx));
    // tasks -----------
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // it should never reach here -----------
    console.printf("main ends.\r\n");
    return 0;
}
