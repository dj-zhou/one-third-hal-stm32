
#include "config.h"

// ============================================================================
void        SystemClock_Config( void );
static void MX_GPIO_Init( void );

// ============================================================================
static void LED_Toggle( void ) {
    HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_6 );
}

int main( void ) {
    utils.systemClockInit();
    MX_GPIO_Init();

    while ( 1 ) {
        LED_Toggle();
        for ( int i = 0; i < 1000; i++ ) {
            for ( int j = 0; j < 1000; j++ ) {
                ;
            }
        }
    }
}

// ============================================================================
static void MX_GPIO_Init( void ) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_6, GPIO_PIN_RESET );

    /*Configure GPIO pin : PC6 */
    GPIO_InitStruct.Pin   = GPIO_PIN_6;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
}

void Error_Handler( void ) {}
