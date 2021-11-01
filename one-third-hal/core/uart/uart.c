#include "uart.h"

#if defined(UART_IS_USED)

// ----------------------------------------------------------------------------
#if defined(STM32F746xx) || defined(STM32F767xx)
void InitUartPins(GPIO_TypeDef* GPIOx_T, uint8_t pin_nT, GPIO_TypeDef* GPIOx_R,
                  uint8_t pin_nR, uint32_t alter) {
    utils.clock.enableGpio(GPIOx_T);
    utils.clock.enableGpio(GPIOx_R);
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin = 1 << pin_nT;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init(GPIOx_T, &GPIO_InitStructure);
    // RX
    GPIO_InitStructure.Pin = 1 << pin_nR;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_R, &GPIO_InitStructure);
}
#endif  // STM32F746xx || STM32F767xx

// ----------------------------------------------------------------------------
void InitUartSettings(UART_HandleTypeDef* huart, uint32_t baud_rate,
                      uint8_t len, char parity, uint8_t stop_b) {

    huart->Init.BaudRate = baud_rate;
    switch (len) {
    case 9:
        // todo: this may need extra attention in IRQ
        huart->Init.WordLength = UART_WORDLENGTH_9B;
        break;
    case 8:
    default:
        huart->Init.WordLength = UART_WORDLENGTH_8B;
        break;
    }

    switch (stop_b) {
    case 2:
        huart->Init.StopBits = UART_STOPBITS_2;
        break;
    case 1:
    default:
        huart->Init.StopBits = UART_STOPBITS_1;
        break;
    }

    switch (parity) {
    case 'O':
    case 'o':
        // todo: this may need extra attention
        huart->Init.Parity = UART_PARITY_ODD;
        break;
    case 'E':
    case 'e':
        // todo: this may need extra attention
        huart->Init.Parity = UART_PARITY_EVEN;
        break;
    case 'N':
    case 'n':
    default:
        huart->Init.Parity = UART_PARITY_NONE;
        break;
    }
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
#if defined(STM32F303xE) || defined(STM32F767xx)
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
#endif

    HAL_UART_Init(huart);
}

// ----------------------------------------------------------------------------
void InitUartNvic(IRQn_Type ch, uint16_t p) {
    HAL_NVIC_SetPriority(ch, p, 0);
    HAL_NVIC_EnableIRQ(ch);
}

// ============================================================================
#endif  // UART_IS_USED
