#include "uart.h"

#if defined(UART5_EXISTS) && defined(UART5_IS_USED)

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t uart5_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_UART5_PC12PD2)
static void InitUart5_PC12PD2(void) {
    // todo
}
#endif  // _USE_UART5_PC12PD2

// ----------------------------------------------------------------------------
static void InitUart5(uint32_t baud, uint8_t data_size, char parity,
                      uint8_t stop) {
    if (_CHECK_BIT(g_config_uart_used, 2)) {
        console.error("%s(): UART5 is uses as the console\r\n");
    }
    _SET_BIT(g_config_uart_used, 2);
    uart5.huart.Instance = UART5;
#if defined(_USE_UART5_PC12PD2)
    InitUart5_PC12PD2();
#endif
    utils.clock.enableUart(uart5.huart.Instance);
    InitUartSettings(&(uart5.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(uart5.huart));
    uart5_uh_mask_ = uart5.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(uart5.huart));

    __HAL_UART_ENABLE_IT(&(uart5.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(uart5.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(UART5_IRQn, 15);
}

// ----------------------------------------------------------------------------
static void Uart5DmaConfig(void) {
    DMA_HandleTypeDef hdma_uart5_rx;
    /* UART5 DMA Init */
    /* UART5_RX Init */
    hdma_uart5_rx.Instance = DMA1_Stream5;
    hdma_uart5_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart5_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart5_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart5_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart5_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart5_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart5_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart5_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_uart5_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_uart5_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Uart5RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_ = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUart5Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(UART5_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Uart5Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(uart5.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Uart5IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void UART5_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart5.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart5.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(uart5.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(uart5.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(uart5.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(uart5.huart));
        Uart5IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(uart5.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t uart5 = {
    .config      = InitUart5             ,
    .dma.config  = Uart5DmaConfig        ,
    .priority    = InitUart5Priority     ,
    .transmit    = Uart5Transmit         ,
    .ring.config = Uart5RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // UART5_EXISTS && UART5_IS_USED
