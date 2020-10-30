#ifndef __CONSOLE_CLI_H
#define __CONSOLE_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart-console.h"

// clang-format off
#if !defined( _CLI_CMD_MAX_LEN )
    #define _CLI_CMD_MAX_LEN            ( 30 )
#endif

#if !defined( _CLI_CMD_MAX_NUM )
    #define _CLI_CMD_MAX_NUM            ( 20 )
#endif

#if !defined( _CLI_HISTORY_CMD_NUM )
    #define _CLI_HISTORY_CMD_NUM        ( 5 )
#endif

#if !defined( _CLI_CMD_PARAM_MAX_NUM )
    #define _CLI_CMD_PARAM_MAX_NUM      ( 20 )
#endif
// clang-format on

typedef struct {
    char*     str;
    CliHandle p;
} CliCmdList_t;

typedef struct {
    char    cmd_buff[_CLI_HISTORY_CMD_NUM][_CLI_CMD_MAX_LEN];
    uint8_t w_index;
} CliHistory_t;

typedef struct {
    int   argc;
    char* cmd_buff_tail;
    char* cmd_buff_cursor;
    char* out_message;
    char* argv[_CLI_CMD_PARAM_MAX_NUM];
    char  argv_buff[_CLI_CMD_MAX_LEN];
    char  cmd_buff[_CLI_CMD_MAX_LEN];
    // history
    CliHistory_t history_cmd;
} Cli_t;

// ============================================================================
// this file is only used by the module uart-console
void CliDeInit( void );
void CliInput( char read_char );
void CliTabCompletion( void );
void CliBackspace( void );
void CliDirection( char read_char );
void CliProcessCmd( char* str );

// ============================================================================
// default CLI commands registered
HAL_StatusTypeDef CliShowCmd( void );
HAL_StatusTypeDef CliReset( void );
HAL_StatusTypeDef CliLogSetLevel( int argc, char** argv );
HAL_StatusTypeDef CliCheckFirmware( void );
HAL_StatusTypeDef CliShowScheduler( int argc, char** argv );

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_CLI_H
