#include "uart.h"

#if defined(UART4_EXISTS) && defined(UART4_IS_USED)

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t uart4_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_UART4_PC10PC11)
static void InitUart4_PC10PC11(void) {
    // todo
}
#endif  // _USE_UART4_PC10PC11

// ----------------------------------------------------------------------------
static void InitUart4(uint32_t baud, uint8_t data_size, char parity,
                      uint8_t stop) {
    if (config_uarts.check(UART4)) {
        uart_error("UART4 is occupied\r\n");
    }
    uart4.huart.Instance = UART4;
#if defined(_USE_UART4_PC10PC11)
    InitUart4_PC10PC11();
#endif
    utils.clock.enableUart(uart4.huart.Instance);
    InitUartSettings(&(uart4.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(uart4.huart));
    uart4_uh_mask_ = uart4.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(uart4.huart));

    __HAL_UART_ENABLE_IT(&(uart4.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(uart4.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(UART4_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void Uart4DmaConfig(void) {
    DMA_HandleTypeDef hdma_uart4_rx;
    /* UART4 DMA Init */
    /* UART4_RX Init */
    hdma_uart4_rx.Instance = DMA1_Stream5;
    hdma_uart4_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart4_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart4_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart4_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_uart4_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart4_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Uart4RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_ = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUart4Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(UART4_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Uart4Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(uart4.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Uart4IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void UART4_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart4.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart4.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(uart4.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart4.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart4.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(uart4.huart));
        Uart4IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(uart4.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t uart4 = {
    .config      = InitUart4             ,
    .dma.config  = Uart4DmaConfig        ,
    .priority    = InitUart4Priority     ,
    .transmit    = Uart4Transmit         ,
    .ring.config = Uart4RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // UART4_EXISTS && UART4_IS_USED
