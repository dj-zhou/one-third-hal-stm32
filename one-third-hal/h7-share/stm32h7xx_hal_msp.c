#include "config.h"

void HAL_MspInit(void) {

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    HAL_NVIC_SetPriority(RCC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RCC_IRQn);
}
