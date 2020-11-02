#include "config.h"

void HAL_MspInit( void ) {

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    // NOJTAG: JTAG-DP Disabled and SW-DP Enabled
    __HAL_AFIO_REMAP_SWJ_NOJTAG();

    HAL_NVIC_SetPriority( RCC_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( RCC_IRQn );
}
