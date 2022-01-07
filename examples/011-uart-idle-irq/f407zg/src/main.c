#include "config.h"
#include <math.h>

uint8_t usart1_dma_buffer[30];
// =============================================================================
void taskPrint(void) {
    for (int i = 0; i < sizeof_array(usart1_dma_buffer); i++) {
        console.printf("%02X ", usart1_dma_buffer[i]);
    }
    console.printf("\r\n");
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
    usart1.dma.config(usart1_dma_buffer, sizeof_array(usart1_dma_buffer));

    // tasks -----------
    stime.scheduler.attach(2000, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf("main ends.\r\n");
    return 0;
}
