#include "core-console.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t level_;
// ============================================================================
#if defined( CONSOLE_IS_USED )

// clang-format off
#if defined( _CONSOLE_USE_USART1_PA9PA10 )
   #define CONSOLE_UART  USART1
#elif defined( _CONSOLE_USE_USART2_PA2PA3 ) || defined( _CONSOLE_USE_USART2_PD5PD6 )
    #define CONSOLE_UART  USART2
#elif defined( _CONSOLE_USE_USART3_PB10PB11 )
    #define CONSOLE_UART  USART3
#elif defined( _CONSOLE_USE_UART4 )
    #define CONSOLE_UART  UART4
#elif defined( _CONSOLE_USE_UART5_PC12PD2 )
    #define CONSOLE_UART  UART5
#else
    #error CONSOLE not defined
#endif

#if (defined _CONSOLE_USE_UART4) || (defined _CONSOLE_USE_UART5_PC12PD2) 
UART_HandleTypeDef hconsole_;
#else 
USART_HandleTypeDef hconsole_;
#endif

#if !defined( _CONSOLE_SIGN_DATA_SIZE )
    #define _CONSOLE_SIGN_DATA_SIZE             50 // > 20
#endif
#if !defined( _CONSOLE_BUFF_LEN )
    #define _CONSOLE_BUFF_LEN                   100 // not used yet
#endif
// clang-format on

// ============================================================================
// common used functions
// ----------------------------------------------------------------------------
static void InitUartPins( GPIO_TypeDef* GPIOx_T, uint8_t pin_nT,
                          GPIO_TypeDef* GPIOx_R, uint8_t pin_nR ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    // TX
    GPIO_InitStructure.Pin = 1 << pin_nT;
    // default setting is push-pull output
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOx_T, &GPIO_InitStructure );
    // RX
    GPIO_InitStructure.Pin   = 1 << pin_nR;
    GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOx_R, &GPIO_InitStructure );
}

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
    hconsole_.Init.Mode = UART_MODE_TX_RX;
#if defined( _CONSOLE_USE_UART4 ) || defined( _CONSOLE_USE_UART5_PC12PD2 )
    hconsole_.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init( &hconsole_ );  // if != HAL_OK ??
#else
    HAL_USART_Init( &hconsole_ );  // if != HAL_OK ??
#endif
}

// ----------------------------------------------------------------------------
void InitUartNvic( uint8_t ch ) {
    HAL_NVIC_EnableIRQ( ch );
    HAL_NVIC_SetPriority( ch, _CONSOLE_PREEMPTION_PRIORITY,
                          _CONSOLE_SUB_PRIORITY );
}

// ============================================================================
static void consoleWriteByte( char data ) {
#if defined( _CONSOLE_USE_UART4 ) || defined( _CONSOLE_USE_UART5_PC12PD2 )
    HAL_UART_Transmit( &hconsole_, ( uint8_t* )&data, 1, 10 );
#else  // to test
    HAL_USART_Transmit( &hconsole_, ( uint8_t* )&data, 1, 10 );
#endif
}

// ============================================================================
static void consoleWriteStr( char* ptr ) {
#if defined( _CONSOLE_USE_UART4 ) || defined( _CONSOLE_USE_UART5_PC12PD2 )
    HAL_UART_Transmit( &hconsole_, ( uint8_t* )ptr, strlen( ptr ), 10 );
#else  // to test
    HAL_USART_Transmit( &hconsole_, ( uint8_t* )ptr, strlen( ptr ), 10 );
#endif
}

// ============================================================================
#if defined( _CONSOLE_USE_USART1_PA9PA10 )
#error _CONSOLE_USE_USART1_PA9PA10: not implemented.
#endif

// ============================================================================
#if defined( _CONSOLE_USE_USART2_PA2PA3 )
static void Init_USART2_PA2PA3( uint32_t baud_rate, uint8_t len, char parity,
                                uint8_t stop_b ) {
    // gpio setting
    utils.enableGpioClock( GPIOA );
    InitUartPins( GPIOA, 2, GPIOA, 3 );
    // usart setting
    utils.enableUartClock( USART2 );
    InitUartSettings( USART2, baud_rate, len, parity, stop_b );

    InitUartNvic( USART2_IRQn );

    __HAL_UART_ENABLE( &hconsole_ );
}
#endif

// ============================================================================
#if defined( _CONSOLE_USE_USART2_PD5PD6 )
// TODO
// STM32F107VCT6 does not have USART2 on these pins
static void Init_USART2_PD5PD6( uint32_t baud_rate, uint8_t len, char parity,
                                uint8_t stop_b ) {
    // gpio setting
    utils.enableGpioClock( GPIOD );
    InitUartPins( GPIOD, 5, GPIOD, 6 );
    // usart setting
    utils.enableUartClock( USART2 );
    // do not forget those how to do this? do not delete
    // RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
    // PinRemapConfig( GPIO_Remap_USART2, ENABLE );
    InitUartSettings( USART2, baud_rate, len, parity, stop_b );

    InitUartNvic( USART2_IRQn );

    __HAL_UART_ENABLE( &hconsole_ );

    // should I do this?
    // USART_ClearITPendingBit( USART2, USART_IT_TC );
}
#endif  // _CONSOLE_USE_USART2_PD5PD6

// ---------------------------------------------------------------------------
#if defined( _CONSOLE_USE_USART2_PA2PA3 ) \
    || defined( _CONSOLE_USE_USART2_PD5PD6 )
void USART2_IRQHandler( void ) {
    // todo, cli must use it
}
#endif  // _CONSOLE_USE_USART2_PD5PD6

// ============================================================================
#if defined( _CONSOLE_USE_USART3_PB10PB11 )
#error _CONSOLE_USE_USART3_PB10PB11: not implemented.
#endif  // _CONSOLE_USE_USART3_PB10PB11

// ============================================================================
#if defined( _CONSOLE_USE_UART4 )
#error _CONSOLE_USE_UART4: not implemented.
#endif

// ============================================================================
#if defined( _CONSOLE_USE_UART5_PC12PD2 )

// ---------------------------------------------------------------------------
// UART5 test on PC12 (TX) & PD2 (RX)
// supported baud rate: 115200, 230400, 460800, 576000, 921600
static void InitUART5_PC12PD2( uint32_t baud_rate, uint8_t len, char parity,
                               uint8_t stop_b ) {
    // gpio setting
    utils.enableGpioClock( GPIOC );
    utils.enableGpioClock( GPIOD );
    InitUartPins( GPIOC, 12, GPIOD, 2 );
    // usart setting
    utils.enableUartClock( UART5 );
    InitUartSettings( UART5, baud_rate, len, parity, stop_b );

    InitUartNvic( UART5_IRQn );

    __HAL_UART_ENABLE( &hconsole_ );
}

// ---------------------------------------------------------------------------
void UART5_IRQHandler( void ) {
    // todo, cli must use it
}
#endif  // _CONSOLE_USE_UART5_PC12PD2

// ============================================================================
static char Seek_Signifier( char** str, char* sign_data, char* sign ) {
    char* fmt    = *str;
    *sign_data++ = '%';
    sign[0]      = '\0';
    sign[1]      = '\0';
    do {
        if ( ( ( *fmt >= 'a' && *fmt <= 'z' )
               || ( *fmt >= 'A' && *fmt <= 'Z' ) )
             && ( *fmt != 'l' ) ) {
            *sign_data++ = *fmt;
            *sign_data   = '\0';
            *str         = fmt + 1;
            if ( sign[0] == '\0' ) {
                sign[0] = *fmt;
            }
            else {
                sign[1] = *fmt;
            }
            return *fmt;
        }
        else if ( *fmt == '%' || *fmt == ' '
                  || ( ( fmt - *str ) >= ( _CONSOLE_SIGN_DATA_SIZE - 3 ) ) ) {
            return '\0';
        }
        else {
            if ( *fmt == 'l' ) {
                sign[0] = 'l';
            }
            *sign_data++ = *fmt++;
        }
    } while ( *fmt );
    return '\0';
}

// ============================================================================
static void printf_b( char* sign_data, unsigned int data ) {
    int bits_number;  // 0 or 1 or _

    if ( *( sign_data + 1 ) == 'b' ) {
        bits_number = 16;
    }
    else {
        *strchr( sign_data, 'b' ) = '\0';
        bits_number               = atoi( sign_data + 1 );
    }

    sign_data[19] = '\0';
    for ( int i = 18; i >= 0; i-- ) {
        if ( ( ( i + 1 ) % 5 ) == 0 ) {
            sign_data[i] = ' ';
        }
        else {
            sign_data[i] = ( data & 0x01 ) | 0x30;
            data         = data >> 1;
        }
    }

    if ( ( ( bits_number / 4 ) > 0 ) && ( ( bits_number / 4 ) <= 4 ) ) {
        consoleWriteStr( "0b " );
        consoleWriteStr( &sign_data[20 - ( ( bits_number >> 2 ) * 5 )] );
    }
}

// ============================================================================
static void printf_f( char* sign_data, double data ) {
    char  buff[_CONSOLE_SIGN_DATA_SIZE];
    int   accuracy = 5, width = 0, index = 0, temp = ( int )data;
    char* str;
    char  fmt[10];

    str = strchr( sign_data, '.' );
    if ( str ) {
        accuracy = atoi( str + 1 );
        width    = atoi( ++sign_data );
        if ( width < 0 ) {
            width = -width;
        }
    }
    index = snprintf( &buff[0], _CONSOLE_SIGN_DATA_SIZE - 1, "%d.", temp );
    temp  = ( ( int )( ( ( double )data - ( double )temp )
                      * ( pow( 10, accuracy ) ) ) );
    if ( temp < 0 ) {
        temp = -temp;
    }
    snprintf( fmt, 9, "%%0%du", accuracy );
    index += snprintf( &buff[index], _CONSOLE_SIGN_DATA_SIZE - 1 - index, fmt,
                       temp );
    if ( index >= width ) {
        consoleWriteStr( buff );
    }
    else {
        index = width - index;
        if ( *sign_data == '-' ) {
            consoleWriteStr( buff );
            while ( index-- ) {
                consoleWriteByte( ' ' );
            }
        }
        else if ( *sign_data == '0' ) {
            while ( index-- ) {
                consoleWriteByte( '0' );
            }
            consoleWriteStr( buff );
        }
        else {
            while ( index-- ) {
                consoleWriteByte( ' ' );
            }
            consoleWriteStr( buff );
        }
    }
}

// ============================================================================
static void printf_d( char* sign_data, int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_ld( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_c( char* sign_data, int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_o( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_u( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_x( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_lx( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_X( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_lX( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_e( char* sign_data, double data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_g( char* sign_data, double data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_s( char* sign_data, char* data ) {
    ( void )sign_data;
    consoleWriteStr( data );
}

// ============================================================================
static void printf_p( char* sign_data, void* data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void printf_n( char* sign_data, int* data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    consoleWriteStr( buff );
}

// ============================================================================
static void console_vs_Printf( char* sign_data, char* format, va_list ap ) {
    char  sign[2];
    char* fmt = format;
    while ( *fmt ) {
        while ( *fmt != '%' && *fmt != '\0' ) {
            consoleWriteByte( *fmt++ );
        }
        if ( *fmt == '%' ) {
            if ( *++fmt == '%' ) {
                consoleWriteByte( *fmt++ );
            }
            else if ( *fmt == '\0' ) {
                consoleWriteByte( '%' );
                break;
            }
            else if ( '\0' != ( Seek_Signifier( &fmt, sign_data, sign ) ) ) {
                switch ( sign[0] ) {
                case 'b': {
                    printf_b( sign_data, va_arg( ap, unsigned int ) );
                } break;
                case 'd': {
                    printf_d( sign_data, va_arg( ap, int ) );
                } break;
                case 'o': {
                    printf_o( sign_data, va_arg( ap, unsigned int ) );
                } break;
                case 'u': {
                    printf_u( sign_data, va_arg( ap, unsigned int ) );
                } break;
                case 'x': {
                    printf_x( sign_data, va_arg( ap, unsigned int ) );
                } break;
                case 'X': {
                    printf_X( sign_data, va_arg( ap, unsigned int ) );
                } break;
                case 'f': {
                    printf_f( sign_data, va_arg( ap, double ) );
                } break;
                case 'e': {
                    printf_e( sign_data, va_arg( ap, double ) );
                } break;
                case 'g': {
                    printf_g( sign_data, va_arg( ap, double ) );
                } break;
                case 'c': {
                    printf_c( sign_data, va_arg( ap, int ) );
                } break;
                case 's': {
                    printf_s( sign_data, va_arg( ap, char* ) );
                } break;
                case 'p': {
                    printf_p( sign_data, va_arg( ap, void* ) );
                } break;
                case 'n': {
                    printf_n( sign_data, va_arg( ap, int* ) );
                } break;
                case 'l': {
                    switch ( sign[1] ) {
                    case 'd': {
                        printf_ld( sign_data, va_arg( ap, long ) );
                    } break;
                    case 'x': {
                        printf_lx( sign_data, va_arg( ap, long ) );
                    } break;
                    case 'X': {
                        printf_lX( sign_data, va_arg( ap, long ) );
                    } break;
                    case 'f': {
                        printf_f( sign_data, va_arg( ap, double ) );
                    } break;
                    default: {
                        consoleWriteStr( RED "\r\n printf(" );
                        consoleWriteStr( sign_data );
                        consoleWriteStr( ") not supported." NOC );
                    }
                    }
                } break;
                default: {
                    consoleWriteStr( "\r\n printf(" );
                    consoleWriteStr( sign_data );
                    consoleWriteStr( ") ERROR" );
                }
                }
            }
            else {
                consoleWriteByte( '%' );
            }
        }
    }
}

// ============================================================================
static void consolePrintf( char* format, ... ) {
    if ( LOG_INFO <= level_ ) {
        char    sign_data[_CONSOLE_SIGN_DATA_SIZE];
        va_list ap;
        va_start( ap, format );
        console_vs_Printf( sign_data, format, ap );
        va_end( ap );
    }
}

// ============================================================================
static void consoleError( char* format, ... ) {
    if ( LOG_INFO <= level_ ) {
        char    sign_data[_CONSOLE_SIGN_DATA_SIZE];
        va_list ap;
        va_start( ap, format );
        while ( 1 ) {
            console_vs_Printf( sign_data, format, ap );
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
static void consoleConfig( uint32_t baud_rate, uint8_t len, char parity,
                           uint8_t stop_b ) {

    level_ = LOG_INFO;
#if defined( _CONSOLE_USE_USART1_PA9PA10 )
    Init_USART1_PA9PA10( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_USART2_PA2PA3 )
    Init_USART2_PA2PA3( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_USART2_PD5PD6 )
    Init_USART2_PD5PD6( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_USART3_PB10PB11 )
    Init_USART3_PB10PB11( baud_rate, len, parity, stop_b );
#elif defined( _CONSOLE_USE_UART4 )
#error console_USART_Config(): _CONSOLE_USE_UART4 not implemented!
#elif defined( _CONSOLE_USE_UART5_PC12PD2 )
    InitUART5_PC12PD2( baud_rate, len, parity, stop_b );
#else
#error console_USART_Config(): not implemented!
#endif

    // used for cli
    // rb.r_ptr = rb.buffer;
    // rb.w_ptr = rb.buffer;
    // setvbuf( stdout, NULL, _IONBF, 0 );  // is this used?

    // command line interface
    // memset( &cmd_, '\0', sizeof( cmd_ ) );
    // syslog.printf( "sizeof( cmd_ ) = %ld\r\n", sizeof( cmd_ ) );
    // syslog.printf( "sizeof( cmd_list_ ) = %ld\r\n", sizeof( cmd_list_ ) );
    // syslog.printf( "sizeof( char* ) = %ld\r\n", sizeof( char* ) );
    // syslog.printf( "sizeof( cli_handle ) = %ld\r\n", sizeof( cli_handle ) );

    // memset( cmd_list_, '\0', sizeof( cmd_list_[100] ) );
    // cmd_.out_message = "\r\nconsole: ";
    // CLI_DeInit();

    // // register some default commands -------------
    // CLI_Register_Cmd( "help", CLI_Cmd_Show );
    // CLI_Register_Cmd( "reboot", CLI_Cmd_Reboot );
    // CLI_Register_Cmd( "syslog &level", CLI_Cmd_Syslog );
    // CLI_Output_Str( cmd_.out_message );

    // console.rxenEnable( true );
}

// ============================================================================
static void consoleTxMode( ConsoleTx_t tx ) {
    GPIO_InitTypeDef GPIO_InitStructure;
    if ( tx == TX_PP ) {
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    }
    else if ( tx == TX_OD ) {
        GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    }
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined( _CONSOLE_USE_USART1_PA9PA10 )
    GPIO_InitStructure.Pin = GPIO_PIN_9;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_USART2_PA2PA3 )
    GPIO_InitStructure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_USART2_PD5PD6 )
    GPIO_InitStructure.Pin = GPIO_PIN_5;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_USART3_PB10PB11 )
    GPIO_InitStructure.Pin = GPIO_PIN_10;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStructure );
#elif defined( _CONSOLE_USE_UART4 )
#error console_TX_Mode(): _CONSOLE_USE_UART4 not implemented!
#elif defined( _CONSOLE_USE_UART5_PC12PD2 )
    GPIO_InitStructure.Pin = GPIO_PIN_12;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStructure );
#else
#error console_TX_Mode(): not implemented!
#endif
}

// ============================================================================
static void consoleEnableRxen( bool enable ) {
#if defined( _CONSOLE_USE_UART4 ) || defined( _CONSOLE_USE_UART5_PC12PD2 )
    if ( enable ) {
        __HAL_UART_ENABLE_IT( &hconsole_, UART_IT_RXNE );
    }
    else {
        __HAL_UART_DISABLE_IT( &hconsole_, UART_IT_RXNE );
    }
#else
    if ( enable ) {
        __HAL_USART_ENABLE_IT( &hconsole_, UART_IT_RXNE );
    }
    else {
        __HAL_USART_DISABLE_IT( &hconsole_, UART_IT_RXNE );
    }
#endif
}

// ============================================================================
// clang-format off
Console_t console = {
    .config     = consoleConfig     ,
    .setTxMode  = consoleTxMode     ,
    .enableRxen = consoleEnableRxen ,
    .printf     = consolePrintf     ,
    .writeByte  = consoleWriteByte  ,
    .writeStr   = consoleWriteStr   ,
    .error      = consoleError     ,
    // todo 
    // .read    = console_vGetChar  ,

    // cli related
    // .registerCmd = CLI_Register_Cmd   ,
    // .process     = CLI_Event_handler  ,
    // .show        = CLI_Cmd_Show       ,
};
// clang-format on

#endif  //  CONSOLE_IS_USED
