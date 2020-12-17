#include "uart.h"

#if defined(USART2_EXISTS) && defined(USART2_IS_USED)

// static

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart2_uh_mask_;
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PA2PA3)
static void InitUsart2_PA2PA3(void) {
    // todo
}
#endif  // _USE_USART2_PA2PA3

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PD5PD6)
static void InitUsart2_PD5PD6(void) {
#if defined(STM32F107xC)
    InitUartPins(GPIOD, 5, GPIOD, 6);  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART2_ENABLE();
#elif defined(STM32F767xx)
    InitUartPins(GPIOD, 5, GPIOD, 6, GPIO_AF7_USART2);  // verifing
#else
#error InitUSART2_PD5PD6(): need to implement and verify!
#endif
}
#endif  // _USE_USART2_PD5PD6

// ----------------------------------------------------------------------------
static void InitUsart2(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (_CHECK_BIT(g_config_uart_used, 2)) {
        console.error("%s(): USART2 is uses as the console\r\n");
    }
    _SET_BIT(g_config_uart_used, 2);
    usart2.huart.Instance = USART2;
#if defined(_USE_USART2_PA2PA3)
    InitUsart2_PA2PA3();  // todo
#elif defined(_USE_USART2_PD5PD6)
    InitUsart2_PD5PD6();
#endif
    utils.clock.enableUart(usart2.huart.Instance);
    InitUartSettings(&(usart2.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart2.huart));
    usart2_uh_mask_ = usart2.huart.Mask;
#endif
    __HAL_UART_ENABLE(&(usart2.huart));

    __HAL_UART_ENABLE_IT(&(usart2.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart2.huart), UART_IT_IDLE);
    // default priority
    InitUartNvic(USART2_IRQn, 15);
}

// ----------------------------------------------------------------------------
static void Usart2DmaConfig(void) {
    DMA_HandleTypeDef hdma_usart2_rx;
    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance                 = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_usart2_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK) {
        //   Error_Handler();
    }
    console.error("%s(): not verified!\r\n", __func__);
}

// ----------------------------------------------------------------------------
static void Usart2RingBufferConfig(uint8_t* data, uint16_t len) {
    usart2.rb = ringbuffer.config(data, len);
}

// ----------------------------------------------------------------------------
static void InitUsart2Priority(uint16_t preempt_p) {
    // if using freeRTOS, the priority cannot be smaller (higher) than 5, todo
    InitUartNvic(USART2_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart2Transmit(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart2.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart2IdleIrqCallback(void) {
    ringbuffer.show(&(usart2.rb), 'H', 9);
}

// ============================================================================
void USART2_IRQHandler(void) {
    uint32_t flag   = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag   = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart2.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart2.huart), &recv, 1, 1000);
        ringbuffer.push(&(usart2.rb), recv);
    }
    // IDLE interrupt -----------------
    flag   = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart2.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart2.huart));
        Usart2IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(usart2.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t usart2 = {
    .config      = InitUsart2             ,
    .dma.config  = Usart2DmaConfig        ,
    .priority    = InitUsart2Priority     ,
    .transmit    = Usart2Transmit         ,
    .ring.config = Usart2RingBufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART2_EXISTS && USART2_IS_USED
