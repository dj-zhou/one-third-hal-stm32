#include "uart.h"

#if defined(USART2_EXISTS) && defined(USART2_IS_USED)

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
static void Uart2RingSetHeader(uint8_t* data, uint8_t len) {
    ringbuffer_set_header(data, len, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart2RingSetLength(uint8_t pos, uint8_t width) {
    ringbuffer_set_length(pos, width, &ring_info);
}

// ----------------------------------------------------------------------------
static uint16_t Uart2RingGetLength(uint8_t* data) {
    return ringbuffer_get_length(data, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart2RingSetType(uint8_t pos, uint8_t width) {
    ringbuffer_set_type(pos, width, &ring_info);
}

// ----------------------------------------------------------------------------
static uint16_t Uart2RingGetType(uint8_t* data) {
    return ringbuffer_get_type(data, &ring_info);
}

// ----------------------------------------------------------------------------
static void Uart2RingSetDevice(RinBufferSearchDevice_e dev) {
    ringbuffer_set_device(dev, &ring_info);
}

// ============================================================================
static DMA_HandleTypeDef hdma_usart2_rx;
static bool usart2_dma_is_used = false;

// ============================================================================
#if defined(STM32F303xE) || defined(STM32F767xx)
uint16_t usart2_uh_mask_;  // is this used?
#endif

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PA2PA3)
// static void Usart2Config_PA2PA3(void) {
//     __HAL_RCC_GPIOA_CLK_ENABLE();
//     GPIO_InitTypeDef GPIO_InitStruct = { 0 };
//     GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
// }
}
#endif  // _USE_USART2_PA2PA3

// ----------------------------------------------------------------------------
#if defined(_USE_USART2_PD5PD6)
static void Usart2Config_PD5PD6(void) {
#if defined(STM32F107xC)
    init_uart_pins(GPIOD, 5, GPIOD, 6);  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART2_ENABLE();
#elif defined(STM32F767xx)
    init_uart_pins(GPIOD, 5, GPIOD, 6, GPIO_AF7_USART2);  // verified
#else
#error Usart2Config_PD5PD6(): need to implement and verify!
#endif
}
#endif  // _USE_USART2_PD5PD6

// ----------------------------------------------------------------------------
static void Usart2Config(uint32_t baud, uint8_t data_size, char parity,
                         uint8_t stop) {
    if (config_uart.check(USART2)) {
        uart_error("USART2 is occupied\r\n");
    }
    usart2.huart.Instance = USART2;
#if defined(_USE_USART2_PA2PA3)
    Usart2Config_PA2PA3();
#elif defined(_USE_USART2_PD5PD6)
    Usart2Config_PD5PD6();
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

    usart2.rb.data = NULL;
    usart2_dma_is_used = false;
}

// ----------------------------------------------------------------------------
static void Usart2RingConfig(uint8_t* data, uint16_t len) {
    usart2.rb = op.ringbuffer.init(data, len);
}

// ----------------------------------------------------------------------------
static void Usart2DmaConfig(uint8_t* buffer, uint16_t len) {
    (void)buffer;
    (void)len;
    // if we use DMA for receiving, we don't want UART_IT_RXNE
    __HAL_UART_DISABLE_IT(&(usart2.huart), UART_IT_RXNE);
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK) {
        // Error_Handler(); todo
    }

    __HAL_LINKDMA(&(usart2.huart), hdmarx, hdma_usart2_rx);
    // this is not needed, otherwise, f767 will get stuck
    // HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
    // HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

#if defined(STM32F407xx)
    // for F407xx, HAL_UART_Receive_DMA() CAN NOT be in front
    // usart2 is tested
    HAL_DMA_Start(&hdma_usart2_rx, (uint32_t) & (usart2.huart.Instance->DR),
                  (uint32_t)buffer, len);
    HAL_UART_Receive_DMA(&(usart2.huart), buffer, len);
#elif defined(STM32F767xx)
    // for F767xx, HAL_UART_Receive_DMA() has to be in front
    // usart2 is tested
    HAL_UART_Receive_DMA(&(usart2.huart), buffer, len);
    HAL_DMA_Start(&hdma_usart2_rx, (uint32_t) & (usart2.huart.Instance->RDR),
                  (uint32_t)buffer, len);
#else
#error "Usart2DmaConfig: not implemented"
#endif

    usart2_dma_is_used = true;
    Usart2RingConfig(buffer, len);
}

// ----------------------------------------------------------------------------
static void Usart2Priority(uint16_t preempt_p) {
    init_uart_nvic(USART2_IRQn, preempt_p);
}

// ----------------------------------------------------------------------------
static void Usart2Send(uint8_t* data, uint16_t size) {
    HAL_UART_Transmit(&(usart2.huart), data, size, 1000);
}

// ============================================================================
static UartMessageNode_t usart2_node[_UART_MESSAGE_NODE_MAX_NUM] = { 0 };
static uint8_t usart2_node_num = 0;

static bool Uart2RingAttach(uint16_t type, uart_irq_hook hook,
                            const char* descr) {
    if (uart_message_attach(type, hook, descr, usart2_node, usart2_node_num)) {
        usart2_node_num++;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
static void Uart2RingShow(void) {
    uart_message_show("USART2", usart2_node, usart2_node_num);
}

// ----------------------------------------------------------------------------
static void Uart2RingCopy(uint8_t* msg, uint8_t* dest, size_t size) {
    // todo: read CRC32 and calculate CRC32 and compare them
    for (size_t i = 0; i < size; i++) {
        *dest++ = msg[ring_info.type_pos + i];
    }
}

// ----------------------------------------------------------------------------
/// warning: this function is only good for one-third's protocol, so it needs to
/// be redefined in projects when that is not one-third's protocol
__attribute__((weak)) void Usart2IdleIrq(void) {
    int8_t search_ret = usart2.ring.search();
    // search returns error code
    if (search_ret < 0) {
        op.ringbuffer.error(search_ret);
    }
    // search returns nothing
    if (search_ret == 0) {
        return;
    }

    uint8_t array[_UART_MESSAGE_MAX_PACKET_SIZE] = { 0 };
    if (usart2.rb.index.dist[0] > _UART_MESSAGE_MAX_PACKET_SIZE) {
        uart_error("%s(): need to make _UART_MESSAGE_MAX_PACKET_SIZE "
                   "larger than %d\r\n",
                   __func__, _UART_MESSAGE_MAX_PACKET_SIZE);
    }
    search_ret = usart2.ring.fetch(array, sizeof_array(array));

    // find the callback function and run it
    uint16_t type = Uart2RingGetType(array);
    for (uint8_t i = 0; i < usart2_node_num; i++) {
        if (usart2_node[i].this_.msg_type == type) {
            usart2_node[i].this_.hook(array);
            return;
        }
    }
    uart_printf("%s(): unknown message type: 0x%04X\r\n", __func__, type);
}

// ----------------------------------------------------------------------------
static void Usart2IdleIrqCallback(void) {
    if (usart2_dma_is_used) {
        // to calculate how many bytes are received
        // PROBLEM: it cannot tell if more than the capacity bytes of data is
        // received
        static uint16_t remaining[2] = { 0, 0 };
        uint16_t receive_count = 0;
        remaining[0] = remaining[1];
        remaining[1] = (uint16_t)(__HAL_DMA_GET_COUNTER(usart2.huart.hdmarx));
        static bool dma_first_packet = true;
        if (dma_first_packet) {
            receive_count = (uint16_t)(usart2.rb.state.capacity - remaining[1]);
            dma_first_packet = false;
            op.ringbuffer.pushN(&(usart2.rb), usart2.rb.data, receive_count);
        }
        else {
            if (remaining[0] > remaining[1]) {
                receive_count = (uint16_t)(remaining[0] - remaining[1]);
            }
            else {
                receive_count = (uint16_t)(usart2.rb.state.capacity
                                           + remaining[0] - remaining[1]);
            }
            // DMA has already pushed data into the ringbuffer
            op.ringbuffer.added(&(usart2.rb), receive_count);
        }
        // debug use, do not delete
        // console.printf("remaining = %d,%d\r\n", remaining[0], remaining[1]);
        // console.printf("receive_count = %d\r\n", receive_count);
        // usart2.ring.show('h', 10);
        // console.printf("usart2.rb.state.count = %d\r\n",
        // usart2.rb.state.count); console.printf("usart2.rb.state.head =
        // %d\r\n", usart2.rb.state.head); console.printf("usart2.rb.state.tail
        // = %d\r\n", usart2.rb.state.tail);
    }
    else {
        // todo: usart2 does not use DMA
    }
    // in either case, we need to notify the system to parse data from the
    // ringbuffer
    Usart2IdleIrq();
}

// ============================================================================
__attribute__((weak)) void USART2_IRQHandler(void) {
    uint32_t flag = 0;
    uint32_t source = 0;
    // RX interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_RXNE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart2.huart), UART_IT_RXNE);
    if (flag != RESET && source != RESET) {
        uint8_t recv;
        HAL_UART_Receive(&(usart2.huart), &recv, 1, 1000);
        if (usart2.rb.data != NULL) {
            op.ringbuffer.push(&(usart2.rb), recv);
        }
    }
    // IDLE interrupt -----------------
    flag = __HAL_UART_GET_FLAG(&(usart2.huart), UART_FLAG_IDLE);
    source = __HAL_UART_GET_IT_SOURCE(&(usart2.huart), UART_IT_IDLE);
    if (flag != RESET && source != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(&(usart2.huart));
        Usart2IdleIrqCallback();
    }
}

// ============================================================================
static void Usart2RingShow(char style, uint16_t width) {
    op.ringbuffer.show(&usart2.rb, style, width);
}

// ----------------------------------------------------------------------------
WARN_UNUSED_RESULT int8_t Usart2Search(void) {
    if (ring_info.device == RINGBUFFER_SEARCH_ONE_THIRD) {
        if ((ring_info.header_len == 0) || (ring_info.len_pos == 0)
            || (ring_info.type_pos == 0)) {
            return 0;
        }
    }
    return op.ringbuffer.search(&usart2.rb, ring_info);
}

// ----------------------------------------------------------------------------
WARN_UNUSED_RESULT int8_t Usart2Fetch(uint8_t* array, uint16_t size) {
    return op.ringbuffer.fetch(&usart2.rb, array, size);
}

// ============================================================================
// clang-format off
UartApi_t usart2 = {
    .config   = Usart2Config  ,
    .priority = Usart2Priority,
    .send     = Usart2Send    ,
    .dma = {
        .config = Usart2DmaConfig,
    },
    .ring = {
        .config = Usart2RingConfig,
        .show   = Usart2RingShow  ,
        .search = Usart2Search    ,
        .fetch  = Usart2Fetch     ,
        .set = {
            .header = Uart2RingSetHeader,
            .length = Uart2RingSetLength,
            .type   = Uart2RingSetType  ,
            .device = Uart2RingSetDevice,
        },
        .get = {
            .length = Uart2RingGetLength,
            .type   = Uart2RingGetType  ,
        },
    },
    .message = {
        .attach = Uart2RingAttach,
        .show   = Uart2RingShow  ,
        .copy   = Uart2RingCopy  ,

    },
};
// clang-format on

// ============================================================================
#endif  // USART2_EXISTS && USART2_IS_USED
