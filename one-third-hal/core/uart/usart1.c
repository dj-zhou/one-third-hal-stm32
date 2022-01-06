#include "uart.h"

#if defined(USART1_EXISTS) && defined(USART1_IS_USED)
// ============================================================================

static RingBuffer_t ring_;
static bool ring_initialized_ = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart1_uh_mask_;  // is this used?
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PA9PA10)
static void InitUsart1_PA9PA10(void) {
#error InitUsart1_PA9PA10(): todo
}
#endif  // _USE_USART1_PA9PA10

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PB6PB7)
static void InitUsart1_PB6PB7(void) {
#error InitUsart1_PB6PB7(): todo
}
#endif  // _USE_USART1_PB6PB7

// ----------------------------------------------------------------------------
static void InitUsart1(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (config_uart.check(USART1)) {
        uart_error("USART1 is occupied\r\n");
    }
    usart1.huart.Instance = USART1;
#if defined(_USE_USART1_PA9PA10)
    InitUsart1_PA9PA10();
#elif defined(_USE_USART1_PB6PB7)
    InitUsart1_PB6PB7();
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
}

// ----------------------------------------------------------------------------
// placeholder, copied from usart1.c
static void Usart1DmaConfig(uint8_t* buffer, uint32_t len) {

    __HAL_UART_DISABLE_IT(&(usart1.huart), UART_IT_RXNE);
    __HAL_RCC_DMA1_CLK_ENABLE();
#if defined(STM32F767xx)
    // disable the DMA
    // DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    // Check if the DMA Stream is effectively disabled
    while ((DMA1_Stream5->CR & DMA_SxCR_EN) != RESET) {
    }
    // Get the CR register value
    uint32_t tmp = DMA1_Stream5->CR;
    // Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC, DIR, CT
    // and DBM bits
    // tmp &= ((uint32_t) ~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST
    //                      | DMA_SxCR_PL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE
    //                      | DMA_SxCR_MINC | DMA_SxCR_PINC | DMA_SxCR_CIRC
    //                      | DMA_SxCR_DIR | DMA_SxCR_CT | DMA_SxCR_DBM));
    // ( void )tmp;
    // Prepare the DMA Stream configuration
    // tmp |= DMA_CHANNEL_4 | DMA_PERIPH_TO_MEMORY | DMA_PINC_DISABLE
    //        | DMA_MINC_ENABLE | DMA_PDATAALIGN_BYTE | DMA_MDATAALIGN_BYTE
    //        | DMA_CIRCULAR | DMA_PRIORITY_VERY_HIGH;

    // Memory burst and peripheral is disabled, no need to config

    // write to register
    // DMA1_Stream5->CR = tmp;

    // Get the DMAy_Streamx FCR value
    // tmp = DMA1_Stream5->FCR;

    // Clear DMDIS and FTH bits
    // tmp &= (uint32_t) ~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);

    // Configure DMAy Streamx FIFO:
    // Set DMDIS bits according to DMA_FIFOMode value
    // Set FTH bits according to DMA_FIFOThreshold value
    // tmp |= DMA_FIFOMODE_DISABLE | DMA_FIFO_THRESHOLD_1QUARTERFULL;

    // Write to DMAy Streamx FCR
    // DMA1_Stream5->FCR = tmp;

    // HAL_UART_Receive_IT(&(usart1.huart), buffer, len);

    // enable the DMA
    // DMA1_Stream5->CR |= DMA_SxCR_EN;
    // HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 6, 0);
    // HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
#endif
}
// ----------------------------------------------------------------------------
static void Usart1RingBufferConfig(uint8_t* data, uint16_t len) {
    usart1.rb = ringbuffer.config(data, len);
}

// ----------------------------------------------------------------------------
static void InitUsart1Priority(uint16_t preempt_p) {
    init_uart_nvic(USART1_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart1Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart1.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart1IdleIrqCallback(void) {
    ringbuffer.show(&(usart1.rb), 'H', 9);
}

// ============================================================================
void USART1_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart1.huart), &recv, 1, 1000);
        ringbuffer.push(&(usart1.rb), recv);
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart1.huart));
        usart1IdleIrqCallback();
    }
    // HAL_UART_IRQHandler(&(usart1.huart));  // what does this do?
}

// ----------------------------------------------------------------------------
// clang-format off
UartApi_t usart1 = {
    .config      = InitUsart1             ,
    .dma.config  = Usart2DmaConfig        ,
    .priority    = InitUsart1Priority     ,
    .send        = Usart1Send             ,
    .ring.config = Usart2RingBufferConfig ,
};
// clang-format on

// ============================================================================
#endif  // USART1_EXISTS && USART1_IS_USED
