#include "config.h"

#include "config-uart.h"

static uint8_t uarts_used = 0;

static uint8_t get_pos(USART_TypeDef* USARTx) {
    uint8_t pos = 0;
    if (USARTx == USART1) {
        pos = 0;
    }
    else if (USARTx == USART2) {
        pos = 1;
    }
#if defined(USART3_EXISTS)
    else if (USARTx == USART3) {
        pos = 2;
    }
#endif
#if defined(UART4_EXISTS)
    else if (USARTx == UART4) {
        pos = 3;
    }
#endif
#if defined(UART5_EXISTS)
    else if (USARTx == UART5) {
        pos = 4;
    }
#endif
#if defined(USART6_EXISTS)
    else if (USARTx == USART6) {
        pos = 5;
    }
#endif
#if defined(UART7_EXISTS)
    else if (USARTx == UART7) {
        pos = 6;
    }
#endif
#if defined(UART8_EXISTS)
    else if (USARTx == UART8) {
        pos = 7;
    }
#endif
    return pos;
}

static void ConfigUartsSet(USART_TypeDef* USARTx, bool value) {
    uint8_t pos = get_pos(USARTx);
    value ? (uarts_used |= 1 << pos) : (uarts_used &= ~(1 << pos));
}

static bool ConfigUartsCheck(USART_TypeDef* USARTx) {
    return _CHECK_BIT(uarts_used, get_pos(USARTx));
}

ConfigUarts_t config_uarts = {
    .set = ConfigUartsSet,
    .check = ConfigUartsCheck,
};
