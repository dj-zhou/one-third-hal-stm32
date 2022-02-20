#include "config.h"

uint8_t usart1_irq_buffer[30];

// =============================================================================
void taskPrint(void) {
    for (int i = 0; i < sizeof_array(usart1_irq_buffer); i++) {
        console.printf("%02X ", usart1_irq_buffer[i]);
    }
    console.printf("\r\n");
    usart1.ring.show('h', 10);
}

// ============================================================================
int main(void) {
    utils.system.initClock(168, 42, 84);
    utils.system.initNvic(4);
    stime.config();
    stime.scheduler.config();
    console.config(2000000);
    console.printf("\r\n-------------- main() starts\r\n");
    led.config(LED_BREATH);

    // setup usart1 and its ringbuffer
    usart1.config(2000000, 8, 'n', 1);
    usart1.ring.show('h', 10);
    // setup ringbuffer, do not use DMA
    usart1.ring.config(usart1_irq_buffer, sizeof_array(usart1_irq_buffer));
    usart1.ring.show('h', 10);

    // tasks -----------
    stime.scheduler.attach(2000, 2, taskPrint, "taskPrint");
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    // main() should never reach to this point -----------
    console.printf("\r\n-------------- main() ends\r\n");
    return 0;
}
