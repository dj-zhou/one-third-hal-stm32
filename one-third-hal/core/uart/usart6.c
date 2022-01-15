#include "uart.h"

#if defined(USART6_EXISTS) && defined(USART6_IS_USED)
// ============================================================================

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart6_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART6_PC6PC7)
static void InitUsart6_PC6PC7(void) {
#error InitUsart6_PC6PC7(): todo
}
#endif  // _USE_USART6_PC6PC7

// ----------------------------------------------------------------------------
#if defined(_USE_USART6_PG14PG9)
static void InitUsart6_PG14PG9(void) {
#error InitUsart6_PG14PG9(): todo
}
#endif  // _USE_USART6_PG14PG9

// ----------------------------------------------------------------------------
static void InitUsart6(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (config_uart.check(USART6)) {
        uart_error("USART6 is occupied\r\n");
    }
    usart6.huart.Instance = USART6;
#if defined(_USE_USART6_PC6PC7)
    InitUsart6_PC6PC7();
#elif defined(_USE_USART6_PG14PG9)
    InitUsart6_PG14PG9();
#endif
    utils.clock.enableUart(usart6.huart.Instance);
    init_uart_settings(&(usart6.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart6.huart));
    usart6_uh_mask_ = usart6.huart.Mask;
#endif

    __HAL_UART_ENABLE(&(usart6.huart));
    __HAL_UART_ENABLE_IT(&(usart6.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart6.huart), UART_IT_IDLE);
    // default priority
    init_uart_nvic(USART6_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
static void Usart6DmaConfig(void) {
    DMA_HandleTypeDef hdma_usart6_rx;
    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA1_Stream5;
    hdma_usart6_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK) {
        //   Error_Handler();
    }
    uart_error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Usart6RingbufferConfig(uint8_t* data, uint16_t len) {
    usart6.rb = ringbuffer.config(data, len);
}

// ----------------------------------------------------------------------------
static void InitUsart6Priority(uint16_t preempt_p) {
    init_uart_nvic(USART6_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart6Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart6.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart6IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void USART6_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart6.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart6.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart6.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart6.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart6.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart6.huart));
        Usart6IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(usart6.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t usart6 = {
    .config      = InitUsart6             ,
    .dma.config  = Usart6DmaConfig        ,
    .priority    = InitUsart6Priority     ,
    .send        = Usart6Send             ,
    .ring.config = Usart6RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART6_EXISTS && USART6_IS_USED
