#include "config-uart.h"

static uint8_t uarts_used = 0;

static uint8_t get_pos(USART_TypeDef* USARTx) {
    uint8_t pos = 0;
    // clang-format off
    switch ((intptr_t)USARTx) {
    case (intptr_t)USART1: pos = 0; break;
    case (intptr_t)USART2: pos = 1; break;
#if defined(USART3_EXISTS)
    case (intptr_t)USART3: pos = 2; break;
#endif
#if defined(UART4_EXISTS)
    case  (intptr_t)UART4: pos = 3; break;
#endif
#if defined(UART5_EXISTS)
    case  (intptr_t)UART5: pos = 4; break;
#endif
#if defined(USART6_EXISTS)
    case (intptr_t)USART6: pos = 5; break;
#endif
#if defined(UART7_EXISTS)
    case  (intptr_t)UART7: pos = 6; break;
#endif
#if defined(UART8_EXISTS)
    case  (intptr_t)UART8: pos = 7; break;
#endif
    default: pos = 0; break;}
    // clang-format on
    return pos;
}

static void ConfigUartet(USART_TypeDef* USARTx, bool value) {
    uint8_t pos = get_pos(USARTx);
    value ? (uarts_used |= ( uint8_t )(1 << pos))
          : (uarts_used &= ( uint8_t )(~(1 << pos)));
}

static bool ConfigUartCheck(USART_TypeDef* USARTx) {
    return _CHECK_BIT(uarts_used, get_pos(USARTx));
}

ConfigUart_t config_uart = {
    .set = ConfigUartet,
    .check = ConfigUartCheck,
};
