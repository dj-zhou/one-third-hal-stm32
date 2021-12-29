#include "uart.h"

#if defined(USART3_EXISTS) && defined(USART3_IS_USED)

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart3_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART3_PB10PB11)
static void InitUsart3_PB10PB11(void) {
    // todo
}
#endif  // _USE_USART3_PB10PB11

// ----------------------------------------------------------------------------
#if defined(_USE_USART3_PC10PC11)
static void InitUsart3_PB10PB11(void) {
    // todo
}
#endif  // _USE_USART3_PC10PC11

// ----------------------------------------------------------------------------
#if defined(_USE_USART3_PD8PD9)
static void InitUsart3_PB10PB11(void) {
    // todo
}
#endif  // _USE_USART3_PD8PD9

// ----------------------------------------------------------------------------
static void InitUsart3(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (config_uarts.check(USART3)) {
        uart_error("USART3 is occupied\r\n");
    }
    usart3.huart.Instance = USART3;
#if defined(_USE_USART3_PB10PB11)
    InitUsart3_PB10PB11();
#endif
    utils.clock.enableUart(usart3.huart.Instance);
    InitUartSettings(&(usart3.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart3.huart));
    usart3_uh_mask_ = usart3.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(usart3.huart));

    __HAL_UART_ENABLE_IT(&(usart3.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart3.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(USART3_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void Usart3DmaConfig(void) {
    DMA_HandleTypeDef hdma_usart3_rx;
    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Stream5;
    hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Usart3RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_ = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUsart3Priority(uint16_t preempt_p) {
    InitUartNvic(USART3_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart3Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart3.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart3IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void USART3_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart3.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart3.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart3.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart3.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart3.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart3.huart));
        Usart3IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(usart3.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t usart3 = {
    .config      = InitUsart3             ,
    .dma.config  = Usart3DmaConfig        ,
    .priority    = InitUsart3Priority     ,
    .transmit    = Usart3Transmit         ,
    .ring.config = Usart3RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART3_EXISTS && USART3_IS_USED
