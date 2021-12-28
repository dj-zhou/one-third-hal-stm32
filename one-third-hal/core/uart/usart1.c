#include "uart.h"

#if defined(USART1_EXISTS) && defined(USART1_IS_USED)
// ============================================================================

#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart1_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PA9PA10)
static void InitUsart1_PA9PA10(void) {
    // todo
}
#endif  // _USE_USART1_PA9PA10

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PB6PB7)
static void InitUsart1_PB6PB7(void) {
    // todo
}
#endif  // _USE_USART1_PB6PB7

// ----------------------------------------------------------------------------
static void InitUsart1(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    usart1.huart.Instance = USART1;
#if defined(_USE_USART1_PA9PA10)
    InitUsart1_PA9PA10();  // todo
#elif defined(_USE_USART1_PB6PB7)
    InitUsart1_PB6PB7();  // todo
#endif
    utils.clock.enableUart(usart1.huart.Instance);
    InitUartSettings(&(usart1.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart1.huart));
    usart1_uh_mask_ = usart1.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(usart1.huart));
    __HAL_UART_ENABLE_IT(&(usart1.huart), UART_IT_RXNE);
    // default priority
    InitUartNvic(USART1_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void InitUsart1Priority(uint16_t preempt_p) {
    InitUartNvic(USART1_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart1Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart1.huart), data, size, 1000);
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t usart1 = {
    .config    = InitUsart1         ,
    .priority  = InitUsart1Priority ,
    .transmit  = Usart1Transmit     ,
};
// clang-format on

// ============================================================================
#endif  // USART1_EXISTS && USART1_IS_USED
