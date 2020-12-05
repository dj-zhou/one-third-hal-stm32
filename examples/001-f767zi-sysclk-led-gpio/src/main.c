
#include "config.h"

// ============================================================================
int main(void) {
    utils.system.initClock(216, 54, 108);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOB, 7, GPIO_MODE_OUTPUT_PP);
    utils.pin.mode(GPIOB, 0, GPIO_MODE_OUTPUT_PP);
    utils.pin.mode(GPIOC, 13, GPIO_MODE_INPUT);  // reset, pull it to HIGH
    utils.pin.pull(GPIOC, 13, GPIO_PULLDOWN);
    while (1) {
        // some delay ------------
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 1000; j++) {
                ;
            }
        }
        if (utils.pin.read(GPIOC, 13)) {
            utils.pin.set(GPIOB, 0, true);
        }
        else {
            utils.pin.set(GPIOB, 0, false);
        }
        utils.pin.toggle(GPIOB, 7);
    }
}
