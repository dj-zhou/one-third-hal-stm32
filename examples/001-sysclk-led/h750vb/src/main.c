
#include "config.h"

// ============================================================================
int main(void) {
    utils.system.initClock(480, 120, 120, 120, 120);
    utils.system.initNvic(4);
    utils.pin.mode(GPIOC, 13, GPIO_MODE_OUTPUT_PP);
    utils.pin.set(GPIOC, 13, false);
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    while (1) {
        // some delay ------------
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 10000; j++) {
                ;
            }
        }
        utils.pin.toggle(GPIOC, 13);
    }
}
