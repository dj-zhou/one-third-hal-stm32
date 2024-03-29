#include "uart.h"

#if defined(USART1_EXISTS) && defined(USART1_IS_USED)

// ============================================================================
static RingBufferInfo_t ring_info = {
    .device = RINGBUFFER_SEARCH_ONE_THIRD,
    .header_len = 0,
    .len_pos = 0,
    .len_width = 0,
    .type_pos = 0,
    .type_width = 0,
};

// ----------------------------------------------------------------------------
static void Uart1RingSetHeader(uint8_t* data, uint8_t len) {
    ringbuffer_set_header(data, len, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart1RingSetLength(uint8_t pos, uint8_t width) {
    ringbuffer_set_length(pos, width, &ring_info);
}

// ----------------------------------------------------------------------------
static uint16_t Uart1RingGetLength(uint8_t* data) {
    return ringbuffer_get_length(data, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart1RingSetType(uint8_t pos, uint8_t width) {
    ringbuffer_set_type(pos, width, &ring_info);
}

// ----------------------------------------------------------------------------
static uint16_t Uart1RingGetType(uint8_t* data) {
    return ringbuffer_get_type(data, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart1RingSetDevice(RinBufferSearchDevice_e dev) {
    ringbuffer_set_device(dev, &ring_info);
}

// ============================================================================
static DMA_HandleTypeDef hdma_usart1_rx;
static bool usart1_dma_is_used = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart1_uh_mask_;  // is this used?
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PA9PA10)
static void Usart1Config_PA9PA10(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
#endif  // _USE_USART1_PA9PA10

// ----------------------------------------------------------------------------
#if defined(_USE_USART1_PB6PB7)
static void Usart1Config_PB6PB7(void) {
#error Usart1Config_PB6PB7(): todo
}
#endif  // _USE_USART1_PB6PB7

// ----------------------------------------------------------------------------
static void Usart1Config(uint32_t baud, uint8_t data_size, char parity,
                         uint8_t stop) {
    if (config_uart.check(USART1)) {
        uart_error("%s(): USART1 is occupied\r\n", __func__);
    }
    usart1.huart.Instance = USART1;
#if defined(_USE_USART1_PA9PA10)
    Usart1Config_PA9PA10();
#elif defined(_USE_USART1_PB6PB7)
    Usart1Config_PB6PB7();
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

    usart1.rb.data = NULL;
    usart1_dma_is_used = false;
}

// ----------------------------------------------------------------------------
static void Usart1RingConfig(uint8_t* data, uint16_t len) {
    usart1.rb = op.ringbuffer.init(data, len);
}

// ----------------------------------------------------------------------------
static void Usart1DmaConfig(uint8_t* buffer, uint16_t len) {
    (void)buffer;
    (void)len;
    // if we use DMA for receiving, we don't want UART_IT_RXNE
    __HAL_UART_DISABLE_IT(&(usart1.huart), UART_IT_RXNE);
    __HAL_RCC_DMA2_CLK_ENABLE();
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
        // Error_Handler(); todo
    }

    __HAL_LINKDMA(&(usart1.huart), hdmarx, hdma_usart1_rx);
    // this is not needed, otherwise, f767 will get stuck
    // HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

#if defined(STM32F407xx)
    // for F407xx, HAL_UART_Receive_DMA() CAN NOT be in front
    // usart1 is tested
    HAL_DMA_Start(&hdma_usart1_rx, (uint32_t) & (usart1.huart.Instance->DR),
                  (uint32_t)buffer, len);
    HAL_UART_Receive_DMA(&(usart1.huart), buffer, len);
#elif defined(STM32F767xx)
    // for F767xx, HAL_UART_Receive_DMA() has to be in front
    // usart1 is NOT tested
    HAL_UART_Receive_DMA(&(usart1.huart), buffer, len);
    HAL_DMA_Start(&hdma_usart1_rx, (uint32_t) & (usart1.huart.Instance->RDR),
                  (uint32_t)buffer, len);
#error "Usart1DmaConfig() STM32F767xx: not tested"
#else
#error "Usart1DmaConfig(): not implemented"
#endif

    usart1_dma_is_used = true;
    Usart1RingConfig(buffer, len);
}

// ----------------------------------------------------------------------------
static void Usart1Priority(uint16_t preempt_p) {
    init_uart_nvic(USART1_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart1Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart1.huart), data, size, 1000);
}

// ============================================================================
static UartMessageNode_t usart1_node[_UART_MESSAGE_NODE_MAX_NUM] = { 0 };
static uint8_t usart1_node_num = 0;

static bool Uart1RingAttach(uint16_t type, uart_irq_hook hook,
                            const char* descr) {
    if (uart_message_attach(type, hook, descr, usart1_node, usart1_node_num)) {
        usart1_node_num++;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
static void Uart1RingShow(void) {
    uart_message_show("USART1", usart1_node, usart1_node_num);
}

// ----------------------------------------------------------------------------
static void Uart1RingCopy(uint8_t* msg, uint8_t* dest, size_t size) {
    // todo: read CRC32 and calculate CRC32 and compare them
    for (size_t i = 0; i < size; i++) {
        *dest++ = msg[ring_info.type_pos + i];
    }
}

// ----------------------------------------------------------------------------
/// warning: this function is only good for one-third's protocol, so it needs to
/// be redefined in projects when that is not one-third's protocol
__attribute__((weak)) void Usart1IdleIrq(void) {
    int8_t search_ret = usart1.ring.search();
    // search returns error code
    if (search_ret < 0) {
        op.ringbuffer.error(search_ret);
    }
    // search returns nothing
    if (search_ret == 0) {
        return;
    }

    uint8_t array[_UART_MESSAGE_MAX_PACKET_SIZE] = { 0 };
    if (usart1.rb.index.dist[0] > _UART_MESSAGE_MAX_PACKET_SIZE) {
        uart_error("%s(): need to make _UART_MESSAGE_MAX_PACKET_SIZE "
                   "larger than %d\r\n",
                   __func__, _UART_MESSAGE_MAX_PACKET_SIZE);
    }
    search_ret = usart1.ring.fetch(array, sizeof_array(array));

    // find the callback function and run it
    uint16_t type = Uart1RingGetType(array);
    for (uint8_t i = 0; i < usart1_node_num; i++) {
        if (usart1_node[i].this_.msg_type == type) {
            usart1_node[i].this_.hook(array);
            return;
        }
    }
    uart_printf("%s(): unknown message type: 0x%04X\r\n", __func__, type);
}

// ----------------------------------------------------------------------------
static void Usart1IdleIrqCallback(void) {
    if (usart1_dma_is_used) {
        // to calculate how many bytes are received
        // PROBLEM: it cannot tell if more than the capacity bytes of data is
        // received
        static uint16_t remaining[2] = { 0, 0 };
        uint16_t receive_count = 0;
        remaining[0] = remaining[1];
        remaining[1] = (uint16_t)(__HAL_DMA_GET_COUNTER(usart1.huart.hdmarx));
        static bool dma_first_packet = true;
        if (dma_first_packet) {
            receive_count = (uint16_t)(usart1.rb.state.capacity - remaining[1]);
            dma_first_packet = false;
            op.ringbuffer.pushN(&(usart1.rb), usart1.rb.data, receive_count);
        }
        else {
            if (remaining[0] > remaining[1]) {
                receive_count = (uint16_t)(remaining[0] - remaining[1]);
            }
            else {
                receive_count = (uint16_t)(usart1.rb.state.capacity
                                           + remaining[0] - remaining[1]);
            }
            // DMA has already pushed data into the ringbuffer
            op.ringbuffer.added(&(usart1.rb), receive_count);
        }
        // debug use, do not delete
        // console.printf("remaining = %d,%d\r\n", remaining[0], remaining[1]);
        // console.printf("receive_count = %d\r\n", receive_count);
        // usart1.ring.show('h', 10);
        // console.printf("usart1.rb.state.count = %d\r\n",
        // usart1.rb.state.count); console.printf("usart1.rb.state.head =
        // %d\r\n", usart1.rb.state.head); console.printf("usart1.rb.state.tail
        // = %d\r\n", usart1.rb.state.tail);
    }
    else {
        // todo: usart1 does not use DMA
    }
    // in either case, we need to notify the system to parse data from the
    // ringbuffer
    Usart1IdleIrq();
}

// ============================================================================
__attribute__((weak)) void USART1_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart1.huart), &recv, 1, 1000);
        if (usart1.rb.data != NULL) {
            op.ringbuffer.push(&(usart1.rb), recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart1.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart1.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart1.huart));
        Usart1IdleIrqCallback();
    }
}

// ============================================================================
static void Usart1RingShow(char style, uint16_t width) {
    op.ringbuffer.show(&usart1.rb, style, width);
}

// ----------------------------------------------------------------------------
WARN_UNUSED_RESULT int8_t Usart1Search(void) {
    if (ring_info.device == RINGBUFFER_SEARCH_ONE_THIRD) {
        if ((ring_info.header_len == 0) || (ring_info.len_pos == 0)
            || (ring_info.type_pos == 0)) {
            return 0;
        }
    }
    return op.ringbuffer.search(&usart1.rb, ring_info);
}

// ----------------------------------------------------------------------------
WARN_UNUSED_RESULT int8_t Usart1Fetch(uint8_t* array, uint16_t size) {
    return op.ringbuffer.fetch(&usart1.rb, array, size);
}

// ============================================================================
// clang-format off
UartApi_t usart1 = {
    .config   = Usart1Config  ,
    .priority = Usart1Priority,
    .send     = Usart1Send    ,
    .dma = {
        .config = Usart1DmaConfig,
    },
    .ring = {
        .config = Usart1RingConfig,
        .show   = Usart1RingShow  ,
        .search = Usart1Search    ,
        .fetch  = Usart1Fetch     ,
        .set = {
            .header = Uart1RingSetHeader,
            .length = Uart1RingSetLength,
            .type   = Uart1RingSetType  ,
            .device = Uart1RingSetDevice,
        },
        .get = {
            .length = Uart1RingGetLength,
            .type   = Uart1RingGetType  ,
        },
    },
    .message = {
        .attach = Uart1RingAttach,
        .show   = Uart1RingShow  ,
        .copy   = Uart1RingCopy  ,

    },
};
// clang-format on

// ============================================================================
#endif  // USART1_EXISTS && USART1_IS_USED
