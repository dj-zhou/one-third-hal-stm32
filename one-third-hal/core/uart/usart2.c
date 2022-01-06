#include "uart.h"

#if defined(USART2_EXISTS) && defined(USART2_IS_USED)
// ============================================================================

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart2_uh_mask_;  // is this used?
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PA2PA3)
static void InitUsart2_PA2PA3(void) {
#error InitUsart2_PA2PA3(): todo
}
#endif  // _USE_USART2_PA2PA3

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PD5PD6)
static void InitUsart2_PD5PD6(void) {
#if defined(STM32F107xC)
    init_uart_pins(GPIOD, 5, GPIOD, 6);  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART2_ENABLE();
#elif defined(STM32F767xx)
    init_uart_pins(GPIOD, 5, GPIOD, 6, GPIO_AF7_USART2);  // verifing
#else
#error InitUSART2_PD5PD6(): need to implement and verify!
#endif
}
#endif  // _USE_USART2_PD5PD6

// ----------------------------------------------------------------------------
static void InitUsart2(uint32_t baud, uint8_t data_size, char parity,
                       uint8_t stop) {
    if (config_uart.check(USART2)) {
        uart_error("USART2 is occupied\r\n");
    }
    usart2.huart.Instance = USART2;
#if defined(_USE_USART2_PA2PA3)
    InitUsart2_PA2PA3();
#elif defined(_USE_USART2_PD5PD6)
    InitUsart2_PD5PD6();
#endif
    utils.clock.enableUart(usart2.huart.Instance);
    init_uart_settings(&(usart2.huart), baud, data_size, parity, stop);
#if defined(STM32F303xE) || defined(STM32F767xx)
    UART_MASK_COMPUTATION(&(usart2.huart));
    usart2_uh_mask_ = usart2.huart.Mask;
#endif

    __HAL_UART_ENABLE(&(usart2.huart));
    __HAL_UART_ENABLE_IT(&(usart2.huart), UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&(usart2.huart), UART_IT_IDLE);
    // default priority
    init_uart_nvic(USART2_IRQn, _UART_PREEMPTION_PRIORITY);
}

// ----------------------------------------------------------------------------
// this is still buggy, doo not use, and do not delete!!
/// reference: function HAL_DMA_Init() in stm32f7xx_hal_dma.c
static void Usart2DmaConfig(uint8_t* buffer, uint32_t len) {

    __HAL_UART_DISABLE_IT(&(usart2.huart), UART_IT_RXNE);
    __HAL_RCC_DMA1_CLK_ENABLE();
#if defined(STM32F767xx)
    // disable the DMA
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    // Check if the DMA Stream is effectively disabled
    while ((DMA1_Stream5->CR & DMA_SxCR_EN) != RESET) {
    }
    // Get the CR register value
    uint32_t tmp = DMA1_Stream5->CR;
    // Clear CHSEL, MBURST, PBURST, PL, MSIZE, PSIZE, MINC, PINC, CIRC, DIR, CT
    // and DBM bits
    tmp &= ((uint32_t) ~(DMA_SxCR_CHSEL | DMA_SxCR_MBURST | DMA_SxCR_PBURST
                         | DMA_SxCR_PL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE
                         | DMA_SxCR_MINC | DMA_SxCR_PINC | DMA_SxCR_CIRC
                         | DMA_SxCR_DIR | DMA_SxCR_CT | DMA_SxCR_DBM));
    ( void )tmp;
    // Prepare the DMA Stream configuration
    tmp |= DMA_CHANNEL_4 | DMA_PERIPH_TO_MEMORY | DMA_PINC_DISABLE
           | DMA_MINC_ENABLE | DMA_PDATAALIGN_BYTE | DMA_MDATAALIGN_BYTE
           | DMA_CIRCULAR | DMA_PRIORITY_VERY_HIGH;

    // Memory burst and peripheral is disabled, no need to config

    // write to register
    DMA1_Stream5->CR = tmp;

    // Get the DMAy_Streamx FCR value
    tmp = DMA1_Stream5->FCR;

    // Clear DMDIS and FTH bits
    tmp &= (uint32_t) ~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);

    // Configure DMAy Streamx FIFO:
    // Set DMDIS bits according to DMA_FIFOMode value
    // Set FTH bits according to DMA_FIFOThreshold value
    tmp |= DMA_FIFOMODE_DISABLE | DMA_FIFO_THRESHOLD_1QUARTERFULL;

    // Write to DMAy Streamx FCR
    DMA1_Stream5->FCR = tmp;

    HAL_UART_Receive_IT(&(usart2.huart), buffer, len);

    // enable the DMA
    DMA1_Stream5->CR |= DMA_SxCR_EN;
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
#endif
}

// ----------------------------------------------------------------------------
static void InitUsart2Priority(uint16_t preempt_p) {
    init_uart_nvic(USART2_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart2Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart2.huart), data, size, 1000);
}

// ============================================================================
// this function should be redefined in projects
__attribute__((weak)) void Usart2IdleIrqCallback(void) {
    // todo
}

// ============================================================================
void USART2_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart2.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart2.huart), &recv, 1, 1000);
        // op.ringbuffer.push(&(usart2.rb), recv);
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_IDLE);
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
    .send        = Usart2Send             ,
};
// clang-format on

// ============================================================================
#endif  // USART2_EXISTS && USART2_IS_USED
