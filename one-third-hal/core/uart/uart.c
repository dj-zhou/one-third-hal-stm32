#include "uart.h"

#if defined(UART_IS_USED)

// ----------------------------------------------------------------------------
#if defined(STM32F746xx) || defined(STM32F767xx)
void init_uart_pins(GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                    GPIO_TypeDef* GPIOx_R, uint8_t pin_nR, uint32_t alter) {
    utils.clock.enableGpio(GPIOx_T);
    utils.clock.enableGpio(GPIOx_R);
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin = (uint32_t)(1 << pin_nT);
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init(GPIOx_T, &GPIO_InitStructure);
    // RX
    GPIO_InitStructure.Pin = (uint32_t)(1 << pin_nR);
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOx_R, &GPIO_InitStructure);
}
#endif  // STM32F746xx || STM32F767xx

// ----------------------------------------------------------------------------
void init_uart_settings(UART_HandleTypeDef* huart, uint32_t baud_rate,
                        uint8_t len, char parity, uint8_t stop_b) {

    huart->Init.BaudRate = baud_rate;
    switch (len) {
    case 9:
        // todo: this may need extra attention in IRQ
        huart->Init.WordLength = UART_WORDLENGTH_9B;
        break;
    case 8:
    default:
        huart->Init.WordLength = UART_WORDLENGTH_8B;
        break;
    }

    switch (stop_b) {
    case 2:
        huart->Init.StopBits = UART_STOPBITS_2;
        break;
    case 1:
    default:
        huart->Init.StopBits = UART_STOPBITS_1;
        break;
    }

    switch (parity) {
    case 'o':
    case 'O':
        // todo: this may need extra attention
        huart->Init.Parity = UART_PARITY_ODD;
        break;
    case 'e':
    case 'E':
        // todo: this may need extra attention
        huart->Init.Parity = UART_PARITY_EVEN;
        break;
    case 'n':
    case 'N':
    default:
        huart->Init.Parity = UART_PARITY_NONE;
        break;
    }
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
#if defined(STM32F303xE) || defined(STM32F767xx)
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
#endif

    HAL_UART_Init(huart);
}

// ----------------------------------------------------------------------------
void init_uart_nvic(IRQn_Type ch, uint16_t p) {
    HAL_NVIC_SetPriority(ch, p, 0);
    HAL_NVIC_EnableIRQ(ch);
}

// ============================================================================
void uart_message_set_header(uint8_t* header, uint8_t len,
                             UartMessageInfo_t* msg_info) {
    if (len > 5) {
        uart_error("%s(): header too long, not supported!\r\n", __func__);
    }
    for (uint8_t i = 0; i < len; i++) {
        msg_info->header[i] = header[i];
    }
    msg_info->header_len = len;
}

// ----------------------------------------------------------------------------
void uart_message_set_length(uint8_t pos, uint8_t width,
                             UartMessageInfo_t* msg_info) {
    if (msg_info->type_pos == pos) {
        uart_error("%s(): type and length cannot be of the same position!\r\n",
                   __func__);
    }
    msg_info->len_pos = pos;
    if (width > 2) {
        uart_error("%s(): too wide for length, not supported!\r\n", __func__);
    }
    msg_info->len_width = width;
}

// ----------------------------------------------------------------------------
uint16_t uart_message_get_length(uint8_t* data, UartMessageInfo_t* msg_info) {
    if ((msg_info->len_pos == 0) || (msg_info->len_width == 0)) {
        return 0;
    }
    uint16_t length = 0;
    uint8_t* length_ptr = (uint8_t*)&length;
    *length_ptr = data[msg_info->len_pos];
    if (msg_info->len_width == 2) {
        length_ptr++;
        *length_ptr = data[msg_info->len_pos + 1];
    }
    return length;
}

// ----------------------------------------------------------------------------
void uart_message_set_type(uint8_t pos, uint8_t width,
                           UartMessageInfo_t* msg_info) {
    if (msg_info->len_pos == pos) {
        uart_error("%s(): type and length cannot be of the same position!\r\n",
                   __func__);
    }
    msg_info->type_pos = pos;
    if (width > 2) {
        uart_error("%s(): too wide for type, not supported!\r\n", __func__);
    }
    msg_info->type_width = width;
}

// ----------------------------------------------------------------------------
uint16_t uart_message_get_type(uint8_t* data, UartMessageInfo_t* msg_info) {
    if ((msg_info->type_pos == 0) || (msg_info->type_width == 0)) {
        return 0;
    }
    uint16_t type = 0;
    uint8_t* type_ptr = (uint8_t*)&type;
    *type_ptr = data[msg_info->type_pos];
    if (msg_info->type_width == 2) {
        type_ptr++;
        *type_ptr = data[msg_info->type_pos + 1];
    }
    return type;
}

// ----------------------------------------------------------------------------
bool uart_message_attach(uint16_t type, uart_irq_hook hook, const char* descr,
                         UartMessageNode_t* node, uint8_t node_num) {
    // you cannot attach too many callbacks
    if (node_num >= _UART_MESSAGE_NODE_MAX_NUM) {
        uart_error("%s(): too many messages attached!\r\n", __func__);
    }
    // you cannot attach two callback functions to one message type (on one
    // port)
    if (node_num > 0) {
        for (uint8_t i = 0; i < node_num; i++) {
            if (node[i].this_.msg_type == type) {
                return false;
            }
        }
    }
    uint8_t len;
    size_t str_len = strlen(descr);
    if (str_len >= _UART_MESSAGE_DESCR_SIZE - 1) {
        len = (uint8_t)(_UART_MESSAGE_DESCR_SIZE - 1);
    }
    else {
        len = (uint8_t)str_len;
    }

    if (node_num == 0) {
        node[0].this_.msg_type = type;
        node[0].this_.hook = hook;
        bzero(node[0].this_.descr, _UART_MESSAGE_DESCR_SIZE);
        strncpy(node[0].this_.descr, descr, len);
        node[0].next_ = NULL;
        return true;
    }

    node[node_num].this_.msg_type = type;
    node[node_num].this_.hook = hook;
    bzero(node[node_num].this_.descr, _UART_MESSAGE_DESCR_SIZE);
    strncpy(node[node_num].this_.descr, descr, len);
    node[node_num - 1].next_ = &node[node_num];
    return true;
}

// ----------------------------------------------------------------------------
void uart_message_show(const char* port, UartMessageNode_t* node,
                       uint8_t node_num) {
    CONSOLE_PRINTF_SEG;
    uart_printk(0, "%s Message Registration | %2d callback", port, node_num);
    if (node_num <= 1) {
        uart_printk(0, "\r\n");
    }
    else {
        uart_printk(0, "s\r\n");
    }
    uart_printk(0, "   type | function\r\n");
    for (uint8_t i = 0; i < node_num; i++) {
        uart_printk(0, " 0X%04X | %s\r\n", node[i].this_.msg_type,
                    node[i].this_.descr);
    }
    CONSOLE_PRINTF_SEG;
}

// ============================================================================
#endif  // UART_IS_USED
