#include "uart.h"

#if defined(UART8_EXISTS) && defined(UART8_IS_USED)

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t uart8_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_UART8_PC10PC11)  // this is wrong, todo
static void InitUart8_PC10PC11(void) {
    // todo
}
#endif  // _USE_UART8_PC10PC11

// ----------------------------------------------------------------------------
static void InitUart8(uint32_t baud, uint8_t data_size, char parity,
                      uint8_t stop) {
    if (config_uarts.check(UART8)) {
        uart_error("UART8 is occupied\r\n");
    }
    uart8.huart.Instance = UART8;
#if defined(_USE_UART8_PC10PC11)
    InitUart8_PC10PC11();
#endif
    utils.clock.enableUart(uart8.huart.Instance);
    InitUartSettings(&(uart8.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(uart8.huart));
    uart8_uh_mask_ = uart8.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(uart8.huart));

    __HAL_UART_ENABLE_IT(&(uart8.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(uart8.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(UART8_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void Uart8DmaConfig(void) {
    DMA_HandleTypeDef hdma_uart8_rx;
    /* UART8 DMA Init */
    /* UART8_RX Init */
    hdma_uart8_rx.Instance = DMA1_Stream5;
    hdma_uart8_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart8_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart8_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart8_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart8_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart8_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart8_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart8_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_uart8_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart8_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Uart8RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_ = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUart8Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(UART8_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Uart8Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(uart8.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Uart8IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void UART8_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart8.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart8.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(uart8.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart8.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart8.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(uart8.huart));
        Uart8IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(uart8.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t uart8 = {
    .config      = InitUart8             ,
    .dma.config  = Uart8DmaConfig        ,
    .priority    = InitUart8Priority     ,
    .transmit    = Uart8Transmit         ,
    .ring.config = Uart8RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // UART8_EXISTS && UART8_IS_USED
