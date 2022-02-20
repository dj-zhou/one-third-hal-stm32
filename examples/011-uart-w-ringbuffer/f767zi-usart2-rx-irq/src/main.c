#include "config.h"

uint8_t usart2_irq_buffer[30];

// =============================================================================
void taskPrint(void) {
    for (int i = 0; i < sizeof_array(usart2_irq_buffer); i++) {
        console.printf("%02X ", usart2_irq_buffer[i]);
    }
    console.printf("\r\n");
    usart2.ring.show('h', 10);
}

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n-------------- main() starts\r\n");
    led.config(LED_BREATH);

    // setup usart2 and its ringbuffer -----------
    usart2.config(2000000, 8, 'n', 1);
    usart2.ring.show('h', 10);
    // setup ringbuffer, do not use DMA -----------
    usart2.ring.config(usart2_irq_buffer, sizeof_array(usart2_irq_buffer));
    usart2.ring.show('h', 10);

    // tasks -----------
    stime.scheduler.attach(2000, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // main() should never reach to this point -----------
    console.printf("\r\n-------------- main() ends\r\n");
    return 0;
}
