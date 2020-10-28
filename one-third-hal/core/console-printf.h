#ifndef __CONSOLE_PRINTF_H
#define __CONSOLE_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart-console.h"
#include <stdarg.h>

// ============================================================================
// this file is only used by the module uart-console

// ============================================================================
void printf_b( char* sign_data, unsigned int data );
void printf_f( char* sign_data, double data );
void printf_d( char* sign_data, int data );
void printf_ld( char* sign_data, long data );
void printf_c( char* sign_data, int data );
void printf_o( char* sign_data, unsigned int data );
void printf_u( char* sign_data, unsigned int data );
void printf_x( char* sign_data, unsigned int data );
void printf_lx( char* sign_data, long data );
void printf_X( char* sign_data, unsigned int data );
void printf_lX( char* sign_data, long data );
void printf_e( char* sign_data, double data );
void printf_g( char* sign_data, double data );
void printf_s( char* sign_data, char* data );
void printf_p( char* sign_data, void* data );
void printf_n( char* sign_data, int* data );
void ConsolePrintf( char* sign_data, char* format, va_list ap );

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_PRINTF_H
