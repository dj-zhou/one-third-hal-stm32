#include "uart.h"

#if defined(UART7_EXISTS) && defined(UART7_IS_USED)

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t uart7_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_UART7_PC10PC11)  // this is wrong, todo
static void InitUart7_PC10PC11(void) {
    // todo
}
#endif  // _USE_UART7_PC10PC11

// ----------------------------------------------------------------------------
static void InitUart7(uint32_t baud, uint8_t data_size, char parity,
                      uint8_t stop) {
    uart7.huart.Instance = UART7;
#if defined(_USE_UART7_PC10PC11)
    InitUart7_PC10PC11();
#endif
    utils.clock.enableUart(uart7.huart.Instance);
    InitUartSettings(&(uart7.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(uart7.huart));
    uart7_uh_mask_ = uart7.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(uart7.huart));

    __HAL_UART_ENABLE_IT(&(uart7.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(uart7.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(UART7_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void Uart7DmaConfig(void) {
    DMA_HandleTypeDef hdma_uart7_rx;
    /* UART7 DMA Init */
    /* UART7_RX Init */
    hdma_uart7_rx.Instance = DMA1_Stream5;
    hdma_uart7_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart7_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart7_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart7_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart7_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart7_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart7_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart7_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_uart7_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart7_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Uart7RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_ = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUart7Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(UART7_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Uart7Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(uart7.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Uart7IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void UART7_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart7.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart7.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(uart7.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart7.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart7.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(uart7.huart));
        Uart7IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(uart7.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t uart7 = {
    .config      = InitUart7             ,
    .dma.config  = Uart7DmaConfig        ,
    .priority    = InitUart7Priority     ,
    .transmit    = Uart7Transmit         ,
    .ring.config = Uart7RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // UART7_EXISTS && UART7_IS_USED
