#include "config.h"

void HAL_MspInit( void ) {

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}
