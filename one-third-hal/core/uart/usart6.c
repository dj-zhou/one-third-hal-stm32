#include "uart.h"

#if defined(USART6_EXISTS) && defined(USART6_IS_USED)

static RingBuffer_t ring_;
static bool         ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart6_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART6_PA2PA3)  // this is wrong, todo
static void InitUsart6_PA2PA3(void) {
    // todo
}
#endif  // _USE_USART6_PA2PA3

// ----------------------------------------------------------------------------
#if defined(_USE_USART6_PD5PD6)
static void InitUsart6_PD5PD6(void) {
#if defined(STM32F107xC)
    InitUartPins(GPIOD, 5, GPIOD, 6);  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART6_ENABLE();
#elif defined(STM32F767xx)
    InitUartPins(GPIOD, 5, GPIOD, 6, GPIO_AF7_USART6);  // verifing
#else
#error InitUSART6_PD5PD6(): need to implement and verify!
#endif
}
#endif  // _USE_USART6_PD5PD6

// ----------------------------------------------------------------------------
static void InitUsart6(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (_CHECK_BIT(g_config_uart_used, 2)) {
        console.error("%s(): USART6 is uses as the console\r\n");
    }
    _SET_BIT(g_config_uart_used, 2);
    usart6.huart.Instance = USART6;
#if defined(_USE_USART6_PA2PA3)
    InitUsart6_PA2PA3();
#endif
    utils.clock.enableUart(usart6.huart.Instance);
    InitUartSettings(&(usart6.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart6.huart));
    usart6_uh_mask_ = usart6.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(usart6.huart));

    __HAL_UART_ENABLE_IT(&(usart6.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart6.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(USART6_IRQn, 15);
}

// ----------------------------------------------------------------------------
static void Usart6DmaConfig(void) {
    DMA_HandleTypeDef hdma_usart6_rx;
    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance                 = DMA1_Stream5;
    hdma_usart6_rx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart6_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_usart6_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Usart6RingbufferConfig(uint8_t* data, uint16_t len) {
    ring_             = RingBufferConfig(data, len);
    ring_initialized_ = true;
}

// ----------------------------------------------------------------------------
static void InitUsart6Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(USART6_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart6Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart6.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart6IdleIrqCallback(void) {
    RingBufferShow(&ring_, 'H', 9);
}

// ============================================================================
void USART6_IRQHandler(void) {
    uint32_t flag   = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag   = __HAL_UART_GET_FLAG(&(usart6.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart6.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart6.huart), &recv, 1, 1000);
        if (ring_initialized_) {
            RingBufferPushOne(&ring_, recv);
        }
    }
    // IDLE interrupt -----------------
    flag   = __HAL_UART_GET_FLAG(&(usart6.huart), UART_FLAG_IDLE);
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
    .transmit    = Usart6Transmit         ,
    .ring.config = Usart6RingbufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART6_EXISTS && USART6_IS_USED
