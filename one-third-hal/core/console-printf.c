#include "console-printf.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// this file is only used by the module uart-console

// ============================================================================
void printf_b( char* sign_data, unsigned int data ) {
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
        console.writeStr( "0b " );
        console.writeStr( &sign_data[20 - ( ( bits_number >> 2 ) * 5 )] );
    }
}

// ============================================================================
void printf_f( char* sign_data, double data ) {
    char  buff[_CONSOLE_SIGN_DATA_SIZE];
    int   accuracy = 5, width = 0, index = 0, temp = ( int )data;
    char* str;
    char  fmt[10];
    // temporary solution ------------------
    if ( ( data > -1 ) && ( data < 0 ) ) {
        console.writeByte( '-' );
    }
    // --------------------
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
        console.writeStr( buff );
    }
    else {
        index = width - index;
        if ( *sign_data == '-' ) {
            console.writeStr( buff );
            while ( index-- ) {
                console.writeByte( ' ' );
            }
        }
        else if ( *sign_data == '0' ) {
            while ( index-- ) {
                console.writeByte( '0' );
            }
            console.writeStr( buff );
        }
        else {
            while ( index-- ) {
                console.writeByte( ' ' );
            }
            console.writeStr( buff );
        }
    }
}

// ============================================================================
void printf_d( char* sign_data, int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_ld( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_c( char* sign_data, int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_o( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_u( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_x( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_lx( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_X( char* sign_data, unsigned int data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_lX( char* sign_data, long data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_e( char* sign_data, double data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_g( char* sign_data, double data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_s( char* sign_data, char* data ) {
    ( void )sign_data;
    console.writeStr( data );
}

// ============================================================================
void printf_p( char* sign_data, void* data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
void printf_n( char* sign_data, int* data ) {
    char buff[_CONSOLE_SIGN_DATA_SIZE];
    snprintf( buff, _CONSOLE_SIGN_DATA_SIZE - 1, sign_data, data );
    console.writeStr( buff );
}

// ============================================================================
static char SeekSignifier( char** str, char* sign_data, char* sign ) {
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
void ConsolePrintf( char* sign_data, char* format, va_list ap ) {
    char  sign[2];
    char* fmt = format;
    while ( *fmt ) {
        while ( *fmt != '%' && *fmt != '\0' ) {
            console.writeByte( *fmt++ );
        }
        if ( *fmt == '%' ) {
            if ( *++fmt == '%' ) {
                console.writeByte( *fmt++ );
            }
            else if ( *fmt == '\0' ) {
                console.writeByte( '%' );
                break;
            }
            else if ( '\0' != ( SeekSignifier( &fmt, sign_data, sign ) ) ) {
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
                        console.writeStr( RED "\r\n printf(" );
                        console.writeStr( sign_data );
                        console.writeStr( ") not supported." NOC );
                    }
                    }
                } break;
                default: {
                    console.writeStr( "\r\n printf(" );
                    console.writeStr( sign_data );
                    console.writeStr( ") ERROR" );
                }
                }
            }
            else {
                console.writeByte( '%' );
            }
        }
    }
}
