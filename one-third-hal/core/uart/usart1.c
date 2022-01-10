#include "uart.h"

#if defined(USART1_EXISTS) && defined(USART1_IS_USED)
// ============================================================================

DMA_HandleTypeDef hdma_usart1_rx;
// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart1_uh_mask_;  // is this used?
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PA9PA10)
static void Usart1Config_PA9PA10(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
#endif  // _USE_USART1_PA9PA10

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PB6PB7)
static void Usart1Config_PB6PB7(void) {
#error Usart1Config_PB6PB7(): todo
}
#endif  // _USE_USART1_PB6PB7

// ----------------------------------------------------------------------------
static void Usart1Config(uint32_t baud, uint8_t data_size, char parity,
                         uint8_t stop) {
    if (config_uart.check(USART1)) {
        uart_error("USART1 is occupied\r\n");
    }
    usart1.huart.Instance = USART1;
#if defined(_USE_USART1_PA9PA10)
    Usart1Config_PA9PA10();
#elif defined(_USE_USART1_PB6PB7)
    Usart1Config_PB6PB7();
#endif
    utils.clock.enableUart(usart1.huart.Instance);
    init_uart_settings(&(usart1.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart1.huart));
    usart1_uh_mask_ = usart1.huart.Mask;
#endif

    __HAL_UART_ENABLE(&(usart1.huart));
    __HAL_UART_ENABLE_IT(&(usart1.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart1.huart), UART_IT_IDLE);
    // default priority
    init_uart_nvic(USART1_IRQn, _UART_PREEMPTION_PRIORITY);

    usart1.rb.data = NULL;
    usart1.dma.is_used = false;
}

// ----------------------------------------------------------------------------
// placeholder, copied from usart1.c
static void Usart1DmaConfig(uint8_t* buffer, uint16_t len) {
    ( void )buffer;
    ( void )len;
    // if we use DMA for receiving, we don't want UART_IT_RXNE
    __HAL_UART_DISABLE_IT(&(usart1.huart), UART_IT_RXNE);
    __HAL_RCC_DMA2_CLK_ENABLE();
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
        // Error_Handler(); todo
    }

    __HAL_LINKDMA(&(usart1.huart), hdmarx, hdma_usart1_rx);
    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

    HAL_DMA_Start(&hdma_usart1_rx, (uint32_t) & (usart1.huart.Instance->DR),
                  ( uint32_t )buffer, len);

    HAL_UART_Receive_DMA(&(usart1.huart), buffer, len);

    usart1.dma.is_used = true;
}

// ----------------------------------------------------------------------------
static void Usart1Priority(uint16_t preempt_p) {
    init_uart_nvic(USART1_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart1Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart1.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart1IdleIrqCallback(void) {
    if (usart1.dma.is_used) {
        console.printf("remaining bytes: %d\r\n",
                       __HAL_DMA_GET_COUNTER(usart1.huart.hdmarx));
    }
    else {
        // todo
    }
}

// ============================================================================
__attribute__((weak)) void USART1_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart1.huart), &recv, 1, 1000);
        if (usart1.rb.data != NULL) {
            op.ringbuffer.push(&(usart1.rb), recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart1.huart));
        Usart1IdleIrqCallback();
    }
}

// ============================================================================
static void Usart1RingConfig(uint8_t* data, uint16_t len) {
    usart1.rb = op.ringbuffer.init(data, len);
}

// ============================================================================
// clang-format off
UartApi_t usart1 = {
    .config      = Usart1Config     ,
    .priority    = Usart1Priority   ,
    .send        = Usart1Send       ,
    .dma.config  = Usart1DmaConfig  ,
    .ring.config = Usart1RingConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART1_EXISTS && USART1_IS_USED
