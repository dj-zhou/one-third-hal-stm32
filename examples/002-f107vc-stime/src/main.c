#include "config.h"

// commnet out and revise the line:
//   utils.pin.toggle( GPIOD, 4 );
// in stime-scheduler.c to check the difference, a GPIO pin will be toggled
// every 1 second
int main(void) {
    utils.system.initClock(72, 36, 72);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOD, 4, GPIO_MODE_OUTPUT_PP);
    stime.config();

    while (1) {
        ;
    }
}
