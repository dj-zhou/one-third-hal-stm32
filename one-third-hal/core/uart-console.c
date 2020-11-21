#include "uart-console.h"
#include "console-cli.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined( _CLI_OUT_MESSAGE )
#define _CLI_OUT_MESSAGE "1/3"
#endif
extern Cli_t    cli_;
extern CliCmd_t cmd_[_CLI_CMD_MAX_NUM];
bool            console_rx_enabled_ = true;

// ============================================================================
#if defined( CONSOLE_IS_USED )

UART_HandleTypeDef hconsole_;

struct {
    char  buffer[_CONSOLE_BUFF_LEN];
    char* r_ptr;  // read pointer
    char* w_ptr;  // write pointer
} rb;

static void ConsoleSetChar( uint8_t* buf, uint32_t len );
// ============================================================================
// CLI related

// ============================================================================
// common used functions
static void ConsoleUartIRQ( void ) {
    uint8_t recv;
    if ( ( __HAL_UART_GET_FLAG( &hconsole_, UART_FLAG_RXNE ) != RESET ) ) {
        HAL_UART_Receive( &hconsole_, &recv, 1, 1000 );
    }
    if ( console_rx_enabled_ ) {
        ConsoleSetChar( &recv, 1 );
    }
}
// ----------------------------------------------------------------------------
#if defined( STM32F030x8 )
static void InitUartPins( GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                          GPIO_TypeDef* GPIOx_R, uint8_t pin_nR,
                          uint32_t alter ) {
    utils.clock.enableGpio( GPIOx_T );
    utils.clock.enableGpio( GPIOx_R );
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin       = 1 << pin_nT;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull      = GPIO_PULLUP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init( GPIOx_T, &GPIO_InitStructure );
    // RX
    GPIO_InitStructure.Pin   = 1 << pin_nR;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;  // this is not GPIO_MODE_INPUT
    GPIO_InitStructure.Pull  = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOx_R, &GPIO_InitStructure );
}
#endif  // STM32F030x8

// ----------------------------------------------------------------------------
#if defined( STM32F103xB ) || defined( STM32F107xC )
static void InitUartPins( GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                          GPIO_TypeDef* GPIOx_R, uint8_t pin_nR ) {

    utils.clock.enableGpio( GPIOx_T );
    utils.clock.enableGpio( GPIOx_R );
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin   = 1 << pin_nT;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOx_T, &GPIO_InitStructure );
    // RX
    GPIO_InitStructure.Pin   = 1 << pin_nR;
    GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOx_R, &GPIO_InitStructure );
}
#endif  // STM32F103xB || STM32F107xC

// ----------------------------------------------------------------------------
#if defined( STM32F407xx ) || defined( STM32F427xx )
static void InitUartPins( GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                          GPIO_TypeDef* GPIOx_R, uint8_t pin_nR,
                          uint32_t alter ) {
    utils.clock.enableGpio( GPIOx_T );
    utils.clock.enableGpio( GPIOx_R );
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin       = 1 << pin_nT;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull      = GPIO_NOPULL;  // or GPIO_PULLUP
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init( GPIOx_T, &GPIO_InitStructure );
    // RX
    GPIO_InitStructure.Pin   = 1 << pin_nR;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;  // or GPIO_PULLUP
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init( GPIOx_R, &GPIO_InitStructure );
}
#endif  // STM32F407xx

// ----------------------------------------------------------------------------
#if defined( STM32F746xx ) || defined( STM32F767xx )
static void InitUartPins( GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                          GPIO_TypeDef* GPIOx_R, uint8_t pin_nR,
                          uint32_t alter ) {
    utils.clock.enableGpio( GPIOx_T );
    utils.clock.enableGpio( GPIOx_R );
    GPIO_InitTypeDef GPIO_InitStructure = { 0 };
    // TX
    GPIO_InitStructure.Pin       = 1 << pin_nT;
    GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull      = GPIO_PULLUP;  // GPIO_NOPULL?
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStructure.Alternate = alter;
    HAL_GPIO_Init( GPIOx_T, &GPIO_InitStructure );
    // RX
    GPIO_InitStructure.Pin   = 1 << pin_nR;
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_PULLUP;  // GPIO_NOPULL?
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init( GPIOx_R, &GPIO_InitStructure );
}
#endif  // STM32F746xx || STM32F767xx

// ----------------------------------------------------------------------------
static void InitUartSettings( USART_TypeDef* USARTx, uint32_t baud_rate,
                              uint8_t len, char parity, uint8_t stop_b ) {

    hconsole_.Instance      = USARTx;
    hconsole_.Init.BaudRate = baud_rate;
    switch ( len ) {
    case 9:
        hconsole_.Init.WordLength = UART_WORDLENGTH_9B;
        break;
    case 8:
    default:
        hconsole_.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    }

    switch ( stop_b ) {
    case 2:
        hconsole_.Init.StopBits = UART_STOPBITS_2;
        break;
    case 1:
    default:
        hconsole_.Init.StopBits = UART_STOPBITS_1;
        break;
    }

    switch ( parity ) {
    case 'O':
    case 'o':
        hconsole_.Init.Parity = UART_PARITY_ODD;
        break;
    case 'E':
    case 'e':
        hconsole_.Init.Parity = UART_PARITY_EVEN;
        break;
    case 'N':
    case 'n':
    default:
        hconsole_.Init.Parity = UART_PARITY_NONE;
        break;
    }
    hconsole_.Init.Mode         = UART_MODE_TX_RX;
    hconsole_.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    hconsole_.Init.OverSampling = UART_OVERSAMPLING_16;
#if defined( STM32F767xx )
    hconsole_.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    hconsole_.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
#endif
    HAL_UART_Init( &hconsole_ );  // if != HAL_OK ??
}

// ----------------------------------------------------------------------------
void InitUartNvic( uint8_t ch ) {
    HAL_NVIC_SetPriority( ch, _CONSOLE_PREEMPTION_PRIORITY,
                          _CONSOLE_SUB_PRIORITY );
    HAL_NVIC_EnableIRQ( ch );
}

// ============================================================================
static void consoleWriteByte( char data ) {
    HAL_UART_Transmit( &hconsole_, ( uint8_t* )&data, 1, 10 );
}

// ============================================================================
static void consoleWriteStr( char* ptr ) {
    HAL_UART_Transmit( &hconsole_, ( uint8_t* )ptr, strlen( ptr ), 10 );
}

// ============================================================================
// no hardware to test it, need to test in the future
#if defined( _CONSOLE_USE_UART1_PA9PA10 ) && defined( USART1_EXISTS )
static void InitUSART1_PA9PA10( uint32_t baud_rate, uint8_t len, char parity,
                                uint8_t stop_b ) {

#if defined( STM32F103xB )
    InitUartPins( GPIOA, 9, GPIOA, 10, 0 );
#elif defined( STM32F107xC )
    InitUartPins( GPIOA, 9, GPIOA, 10 );
#error no hardware to verify yet
#elif defined( STM32F407xx )
    InitUartPins( GPIOA, 9, GPIOA, 10, GPIO_AF7_USART1 );  // to verify
#else
#error InitUSART1_PA9PA10(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART1 );
    InitUartSettings( USART1, baud_rate, len, parity, stop_b );

    InitUartNvic( USART1_IRQn );

    // __HAL_UART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART1_PA9PA10  && defined( USART1_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART1_PB6PB7 ) && defined( USART1_EXISTS )
static void InitUSART1_PB6PB7( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOB, 6, GPIOB, 7 );
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART1_ENABLE();
#else
#error InitUSART1_PB6PB7(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART1 );
    InitUartSettings( USART1, baud_rate, len, parity, stop_b );

    InitUartNvic( USART1_IRQn );

    // __HAL_UART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART1_PA9PA10

// ---------------------------------------------------------------------------
#if defined( _CONSOLE_USE_UART1_PA9PA10 ) \
    || defined( _CONSOLE_USE_UART1_PB6PB7 )
#if defined( USART1_EXISTS )
void USART1_IRQHandler( void ) {
#if defined( STM32F107xC )
    ConsoleUartIRQ();
#else
#error USART1_IRQHandler(): need to implement and verify!
#endif
}
#endif
#endif  // _CONSOLE_USE_UART1_PA9PA10 || _CONSOLE_USE_UART1_PB6PB7

// ============================================================================
#if defined( _CONSOLE_USE_UART2_PA2PA3 ) && defined( USART2_EXISTS )
static void InitUSART2_PA2PA3( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {

#if defined( STM32F030x8 )
    InitUartPins( GPIOA, 2, GPIOA, 3, GPIO_AF1_USART2 );  // verified
#elif defined( STM32F103xB )
    InitUartPins( GPIOA, 2, GPIOA, 3 );                   // verified
#elif defined( STM32F107xC )
    InitUartPins( GPIOA, 2, GPIOA, 3 );                    // verified
#elif defined( STM32F407xx )
    InitUartPins( GPIOA, 2, GPIOA, 3, GPIO_AF7_USART2 );  // verified
#else
#error InitUSART2_PA2PA3(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART2 );
    InitUartSettings( USART2, baud_rate, len, parity, stop_b );

    InitUartNvic( USART2_IRQn );

    // __HAL_UART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART2_PA2PA3 && defined( USART2_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART2_PD5PD6 ) && defined( USART2_EXISTS )
// TODO
// STM32F107VCT6 does not have USART2 on these pins
static void InitUSART2_PD5PD6( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOD, 5, GPIOD, 6 );  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART2_ENABLE();
#else
#error InitUSART2_PD5PD6(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART2 );
    InitUartSettings( USART2, baud_rate, len, parity, stop_b );

    // these may work only for F103, F107, etc, will fix

    InitUartNvic( USART2_IRQn );

    // __HAL_UART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART2_PD5PD6 && defined( USART2_EXISTS )

// ---------------------------------------------------------------------------
#if defined( _CONSOLE_USE_UART2_PA2PA3 ) || defined( _CONSOLE_USE_UART2_PD5PD6 )
#if defined( USART2_EXISTS )
void USART2_IRQHandler( void ) {

// STM32F030x8 is verified
// STM32F103xB is verified
// STM32F107xC is verified
#if defined( STM32F030x8 ) || defined( STM32F103xB ) || defined( STM32F107xC ) \
    || defined( STM32F407xx )
    ConsoleUartIRQ();
#else
#error USART2_IRQHandler(): need to implement and verify!
#endif
}
#endif
#endif  // _CONSOLE_USE_UART2_PA2PA3 || _CONSOLE_USE_UART2_PD5PD6

// ============================================================================
#if defined( _CONSOLE_USE_UART3_PB10PB11 ) && defined( USART3_EXISTS )
static void InitUSART3_PB10PB11( uint32_t baud_rate, uint8_t len, char parity,
                                 uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOB, 10, GPIOB, 11 );  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
#else
#error InitUSART3_PB10PB11(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART3 );
    InitUartSettings( USART3, baud_rate, len, parity, stop_b );

    // these may work only for F103, F107, etc, will fix
    InitUartNvic( USART3_IRQn );

    // __HAL_USART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART3_PB10PB11 && defined( USART3_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART3_PC10PC11 ) && defined( USART3_EXISTS )
static void InitUSART3_PC10PC11( uint32_t baud_rate, uint8_t len, char parity,
                                 uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOC, 10, GPIOC, 11 );  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART3_PARTIAL();
#else
#error InitUSART3_PC10PC11(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( USART3 );
    InitUartSettings( USART3, baud_rate, len, parity, stop_b );

    InitUartNvic( USART3_IRQn );

    // __HAL_USART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART3_PC10PC11 && defined( USART3_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART3_PD8PD9 ) && defined( USART3_EXISTS )
static void InitUSART3_PD8PD9( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOD, 8, GPIOD, 9 );  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART3_ENABLE();
#elif defined( STM32F746xx ) || defined( STM32F767xx )
    InitUartPins( GPIOD, 8, GPIOD, 9, GPIO_AF7_USART3 );  // verified
#else
#error InitUSART3_PD8PD9(): need to implement and verify!
#endif

    // usart setting
    utils.clock.enableUart( USART3 );
    InitUartSettings( USART3, baud_rate, len, parity, stop_b );

    InitUartNvic( USART3_IRQn );

    // __HAL_UART_ENABLE( &hconsole_ );
}
#endif  // _CONSOLE_USE_UART3_PD8PD9 && defined( USART3_EXISTS )

// ---------------------------------------------------------------------------
#if defined( _CONSOLE_USE_UART3_PB10PB11 )    \
    || defined( _CONSOLE_USE_UART3_PC10PC11 ) \
    || defined( _CONSOLE_USE_UART3_PD8PD9 )
#if defined( USART3_EXISTS )
void USART3_IRQHandler( void ) {
#if defined( STM32F107xC )
    ConsoleUartIRQ();
#elif defined( STM32F746xx )
    ConsoleUartIRQ();
#elif defined( STM32F767xx )
    ConsoleUartIRQ();
#else
#error USART3_IRQHandler(): need to implement and verify!
#endif
}
#endif
#endif  // _CONSOLE_USE_UART3_PB10PB11 || _CONSOLE_USE_UART3_PC10PC11 ||
        // _CONSOLE_USE_UART3_PD8PD9

// ============================================================================
#if defined( _CONSOLE_USE_UART4_PC10PC11 ) && defined( UART4_EXISTS )
static void InitUART4_PC10PC11( uint32_t baud_rate, uint8_t len, char parity,
                                uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOC, 10, GPIOC, 11 );  // verified
    __HAL_RCC_AFIO_CLK_ENABLE();
#else
#error InitUART4_PC10PC11(): need to implement and verify!
#endif

    // usart setting
    utils.clock.enableUart( UART4 );
    InitUartSettings( UART4, baud_rate, len, parity, stop_b );

    // these may work only for F103, F107, etc, will fix

    InitUartNvic( UART4_IRQn );
}

// ---------------------------------------------------------------------------
void UART4_IRQHandler( void ) {
#if defined( STM32F107xC )
    ConsoleUartIRQ();
#else
#error UART4_IRQHandler(): need to implement and verify!
#endif
}
#endif  // _CONSOLE_USE_UART4_PC10PC11 && defined( UART4_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART5_PC12PD2 ) && defined( UART5_EXISTS )

// ---------------------------------------------------------------------------
// UART5 test on PC12 (TX) & PD2 (RX)
// supported baud rate: 115200, 230400, 460800, 576000, 921600
static void InitUART5_PC12PD2( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {

#if defined( STM32F107xC )
    InitUartPins( GPIOC, 12, GPIOD, 2 );  // verified
#else
#error InitUART5_PC12PD2(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( UART5 );
    InitUartSettings( UART5, baud_rate, len, parity, stop_b );

    InitUartNvic( UART5_IRQn );
}

// ---------------------------------------------------------------------------
void UART5_IRQHandler( void ) {
#if defined( STM32F107xC )
    ConsoleUartIRQ();
#else
#error UART5_IRQHandler(): need to implement and verify!
#endif
}
#endif  // _CONSOLE_USE_UART5_PC12PD2 && defined( UART5_EXISTS )

// ============================================================================
#if defined( _CONSOLE_USE_UART7_PE8PE7 ) && defined( UART7_EXISTS )
static void InitUART7_PE8PE7( uint32_t baud_rate, uint8_t len, char parity,
                              uint8_t stop_b ) {

#if defined( STM32F427xx )
    InitUartPins( GPIOE, 8, GPIOE, 7, GPIO_AF8_UART7 );  // verified
#else
#error InitUART7_PE8PE7(): need to implement and verify!
#endif
    // usart setting
    utils.clock.enableUart( UART7 );
    InitUartSettings( UART7, baud_rate, len, parity, stop_b );

    InitUartNvic( UART7_IRQn );
}

// ---------------------------------------------------------------------------
void UART7_IRQHandler( void ) {
#if defined( STM32F427xx )
    ConsoleUartIRQ();
#else
#error UART7_IRQHandler(): need to implement and verify!
#endif
}
#endif  // _CONSOLE_USE_UART7_PE8PE7 && defined( UART7_EXISTS )

// ============================================================================
extern void ConsolePrintf( char* sign_data, char* format, va_list ap );

// ============================================================================
static void oneThirdPrintk( LogLevel_e level, char* format, ... ) {
    if ( level <= console.level ) {
        char    sign_data[_CONSOLE_SIGN_DATA_SIZE];
        va_list ap;
        va_start( ap, format );
        ConsolePrintf( sign_data, format, ap );
        va_end( ap );
    }
}

// ============================================================================
static void oneThirdPrintf( char* format, ... ) {
    if ( LOG_INFO <= console.level ) {
        char    sign_data[_CONSOLE_SIGN_DATA_SIZE];
        va_list ap;
        va_start( ap, format );
        ConsolePrintf( sign_data, format, ap );
        va_end( ap );
    }
}

// ============================================================================
static void consoleError( char* format, ... ) {
    if ( LOG_INFO <= console.level ) {
        char    sign_data[_CONSOLE_SIGN_DATA_SIZE];
        va_list ap;
        va_start( ap, format );
        while ( 1 ) {
            ConsolePrintf( sign_data, format, ap );
            // just some delay
            for ( int i = 0; i < 3000; i++ ) {
                for ( int j = 0; j < 3000; j++ ) {
                    ;
                }
            }
        }
        va_end( ap );
    }
}

// ============================================================================
static char consoleGetChar( uint16_t time ) {
    char read_char;
    do {
        if ( rb.r_ptr != rb.w_ptr ) {
            read_char = *rb.r_ptr;
            if ( ++rb.r_ptr == ( rb.buffer + _CONSOLE_BUFF_LEN ) ) {
                rb.r_ptr = rb.buffer;
            }
            return read_char;
        }
    } while ( time-- != 0 );

    return 0;
}

// ============================================================================
static void consoleSetLevel( LogLevel_e l ) {
    console.level = l;
}

// ============================================================================
static void ConsoleSetChar( uint8_t* buf, uint32_t len ) {
    do {
        if ( rb.w_ptr >= rb.r_ptr ) {
            if ( rb.w_ptr + len <= ( rb.buffer + _CONSOLE_BUFF_LEN ) ) {
                strncpy( rb.w_ptr, ( char* )buf, ( uint16_t )len );
                rb.w_ptr = rb.w_ptr + len;
                buf      = buf + len;
                len      = 0;
                if ( rb.w_ptr == ( rb.buffer + _CONSOLE_BUFF_LEN ) ) {
                    rb.w_ptr = rb.buffer;
                }
            }
            else {
                strncpy( rb.w_ptr, ( char* )buf,
                         ( uint16_t )( ( rb.buffer + _CONSOLE_BUFF_LEN )
                                       - rb.w_ptr ) );
                buf      = buf + ( rb.buffer + _CONSOLE_BUFF_LEN - rb.w_ptr );
                len      = len - ( rb.buffer + _CONSOLE_BUFF_LEN - rb.w_ptr );
                rb.w_ptr = rb.buffer;
            }
        }
        else {
            if ( ( ( rb.r_ptr - 1 ) - rb.w_ptr ) >= len ) {
                strncpy( rb.w_ptr, ( char* )buf, ( uint16_t )len );
                buf      = buf + len;
                rb.w_ptr = rb.w_ptr + len;
                len      = 0;
            }
            else {
                strncpy( rb.w_ptr, ( char* )buf,
                         ( uint16_t )( ( rb.r_ptr - 1 ) - rb.w_ptr ) );
                buf      = buf + ( ( rb.r_ptr - 1 ) - rb.w_ptr );
                len      = 0;
                rb.w_ptr = rb.r_ptr - 1;
            }
        }
    } while ( len != 0 );
}

// ============================================================================
static uint8_t cli_cmd_num_ = 0;
static void    CliAttachCmd( char* str, CliHandle p ) {
    if ( cli_cmd_num_ < _CLI_CMD_MAX_NUM - 1 ) {
        cmd_[cli_cmd_num_].str = str;
        cmd_[cli_cmd_num_].p   = p;
        cli_cmd_num_++;
    }
    else {
        console.error(
            "Too many cli commands were registered. \r\nYou can "
            "increase it by defining _CLI_CMD_MAX_NUM in config.h\r\n" );
    }
}

// ============================================================================
// HAL_StatusTypeDef CliSuspend( int argc, char** argv );
// -----------------------------
static void consoleConfig( uint32_t baud_rate, uint8_t len, char parity,
                           uint8_t stop_b ) {

    console.level = LOG_INFO;
    rb.r_ptr      = rb.buffer;
    rb.w_ptr      = rb.buffer;

#if defined( _CONSOLE_USE_UART1_PA9PA10 )
    g_config_usart_used |= 1 << 1;
    InitUSART1_PA9PA10( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART1_PB6PB7 )
    g_config_usart_used |= 1 << 1;
    InitUSART1_PB6PB7( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART2_PA2PA3 )
    g_config_usart_used |= 1 << 2;
    InitUSART2_PA2PA3( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART2_PD5PD6 )
    g_config_usart_used |= 1 << 2;
    InitUSART2_PD5PD6( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART3_PB10PB11 )
    g_config_usart_used |= 1 << 3;
    InitUSART3_PB10PB11( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART3_PC10PC11 )
    g_config_usart_used |= 1 << 3;
    InitUSART3_PC10PC11( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART3_PD8PD9 )
    g_config_usart_used |= 1 << 3;
    InitUSART3_PD8PD9( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART4_PC10PC11 )
    g_config_usart_used |= 1 << 4;
    InitUART4_PC10PC11( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART5_PC12PD2 )
    g_config_usart_used |= 1 << 5;
    InitUART5_PC12PD2( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART7_PE8PE7 )
    g_config_usart_used |= 1 << 7;
    InitUART7_PE8PE7( baud_rate, len, parity, stop_b );
#else
#error consoleConfig(): not implemented!
#endif
    __HAL_UART_ENABLE( &hconsole_ );

    // command line interface -------------
    memset( &cli_, '\0', sizeof( cli_ ) );
    memset( cmd_, '\0', sizeof( cmd_[100] ) );
    cli_.out_message = "\r\n" CYN _CLI_OUT_MESSAGE NOC ": ";
    CliDeInit();

    // register some default commands -------------
    CliAttachCmd( "help", ( CliHandle )CliShowCmd );
    CliAttachCmd( "system &cmd", ( CliHandle )CliSystem );
    CliAttachCmd( "log &level", ( CliHandle )CliLogSetLevel );
    CliAttachCmd( "scheduler &cmd", ( CliHandle )CliShowScheduler );

#if defined( _STIME_USE_SCHEDULER )
    CliAttachCmd( "cli-suspend &seconds", ( CliHandle )CliSuspend );
#endif

    console.enableRxen( true );
}

// ============================================================================
static void consoleTxMode( ConsoleTx_e tx ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    if ( tx == TX_PP ) {
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    }
    else if ( tx == TX_OD ) {
        GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    }
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined( _CONSOLE_USE_UART1_PA9PA10 )
    GPIO_InitStructure.Pin = GPIO_PIN_9;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART1_PB6PB7 )
    GPIO_InitStructure.Pin = GPIO_PIN_6;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART2_PA2PA3 )
    GPIO_InitStructure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART2_PD5PD6 )
    GPIO_InitStructure.Pin = GPIO_PIN_5;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART3_PB10PB11 )
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART3_PC10PC11 )
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART3_PD8PD9 )
    GPIO_InitStructure.Pin = GPIO_PIN_8;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART4_PC10PC11 )
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART5_PC12PD2 )
    GPIO_InitStructure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART7_PE8PE7 )
    GPIO_InitStructure.Pin = GPIO_PIN_8;
    HAL_GPIO_Init( GPIOE, &GPIO_InitStructure );
#else
#error console_TX_Mode(): not implemented!
#endif
}

// ============================================================================
static void consoleEnableRxen( bool enable ) {
    if ( enable ) {
        __HAL_UART_ENABLE_IT( &hconsole_, UART_IT_RXNE );
    }
    else {
        __HAL_UART_DISABLE_IT( &hconsole_, UART_IT_RXNE );
    }
}

// ============================================================================
static void consoleSetRxStatus( bool v ) {
    console_rx_enabled_ = v;
}

// ============================================================================
static bool consoleGetRxStatus( void ) {
    return console_rx_enabled_;
}

// ============================================================================
// some values:
// \n: 0d10
// \r: 0d13
// keys:
// ENTER    : 0d13 (0x0D)
// ESC      : 0d27 (0x1B)
// Tab      : 0d09 (0x09)
// backspace: 0d127 (0x7F)
// space    : 0d32 (0x20)

// 0x1B then 0x5B:
// up arrow: 0d27, then 0d91 (0x1B, then 0x5B)
// down arrow: 0d27, then 0d91 (0x1B, then 0x5B), why it is the same as up
//             arrow?
// left/right arrows: the same
// home, end, page up, page down, delete

static void CliProcess( void ) {
    char           read_char;
    static uint8_t label = 0;

    while ( ( read_char = console.read( 0 ) ) != 0 ) {
        if ( read_char == '\n' || read_char == '\r' ) {
            label = 0;
            if ( ( cli_.cmd_buff == cli_.cmd_buff_tail )
                 || ( cli_.cmd_buff[0] == ' ' ) ) {
                ;  // just do nothing
            }
            else {
                if ( *( --cli_.cmd_buff_tail ) == ' ' ) {
                    *cli_.cmd_buff_tail = '\0';
                }
                CliProcessCmd( cli_.cmd_buff );
                CliDeInit();
            }
            console.writeStr( cli_.out_message );
        }
        else {
            if ( read_char == 0x09 ) {
                CliTabCompletion( console.read( 0xffff ) );
            }
            // 0x7F is the key value of backspace on Linux
            else if ( read_char == 0x7F ) {
                CliBackspace();
            }
            else if ( ( read_char >= '\b' ) && ( read_char < ' ' ) ) {
                label = 0;

                if ( read_char == 0x1b && console.read( 0xffff ) == 0x5b ) {
                    CliDirection( console.read( 0xffff ) );
                }
            }
            else if ( ( read_char >= ' ' ) && ( read_char <= '~' ) ) {
                if ( read_char == '#' || label == 1 ) {
                    label = true;
                }
                else if ( ( cli_.cmd_buff_tail - cli_.cmd_buff )
                          < ( _CLI_CMD_MAX_LEN - 1 ) ) {
                    CliInput( read_char );
                }
            }
        }
    }
}

// ============================================================================
// clang-format off
Console_t console = {
    .level       = LOG_INFO          ,
    .config      = consoleConfig     ,
    .setTxMode   = consoleTxMode     ,
    .enableRxen  = consoleEnableRxen ,
    .setRxStatus = consoleSetRxStatus,
    .getRxStatus = consoleGetRxStatus,
    .printk      = oneThirdPrintk    ,
    .printf      = oneThirdPrintf    ,
    .error       = consoleError      ,
    .writeByte   = consoleWriteByte  ,
    .writeStr    = consoleWriteStr   ,
    .read        = consoleGetChar    ,

    // cli related
    .cli.setLevel = consoleSetLevel,
    .cli.attach   = CliAttachCmd   ,
    .cli.process  = CliProcess     ,
};
// clang-format on

#endif  //  CONSOLE_IS_USED
