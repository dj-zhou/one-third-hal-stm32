#include "console-cli.h"
#include <stdlib.h>
#include <string.h>

Cli_t        cli_;
CliCmdList_t cmd_list_[_CLI_CMD_MAX_NUM];

// ============================================================================
void CliDeInit() {
    cli_.cmd_buff_tail   = cli_.cmd_buff;
    cli_.cmd_buff_cursor = cli_.cmd_buff;
    cli_.cmd_buff[0]     = '\0';
}

// ============================================================================
static char* CliGetParam( char* str, uint8_t num ) {
    char*   dst = str;
    uint8_t i   = 0;
    while ( *dst == ' ' ) {
        dst++;
    }
    if ( num == 0 && *dst ) {
        return dst;
    }
    while ( *dst ) {
        if ( *dst == '[' ) {
            dst = strchr( dst, ']' ) + 1;
            if ( dst == NULL ) {
                console.writeStr( cli_.out_message );
                console.writeStr( "instruction standard error!" );
                return NULL;
            }
        }
        else if ( *dst++ == ' ' ) {
            while ( *dst == ' ' )
                dst++;
            i++;
            if ( i == num && *dst ) {
                return dst;
            }
        }
    }
    return NULL;
}

// ============================================================================
static void CliFormatCmd( char* strtop, char** parameter ) {
    char* strend;

    cli_.argc    = 0;
    cli_.argv[0] = cli_.argv_buff;

    do {
        strend = strchr( strtop, ' ' );
        if ( strend == NULL ) {
            if ( strncmp( strtop, "-all", 4 ) ) {
                strcpy( cli_.argv[cli_.argc], strtop );
                *( cli_.argv[cli_.argc++] + strlen( strtop ) ) = '\0';
                cli_.argv[cli_.argc] =
                    cli_.argv[cli_.argc - 1] + strlen( strtop ) + 1;
            }
            else {
                *parameter = strtop + 1;
                parameter++;
            }
        }
        else {
            if ( strncmp( strtop, "-all", 4 ) ) {
                strncpy( cli_.argv[cli_.argc], strtop, strend - strtop );
                *( cli_.argv[cli_.argc++] + ( strend - strtop ) ) = '\0';
                cli_.argv[cli_.argc] =
                    cli_.argv[cli_.argc - 1] + ( strend - strtop ) + 1;
            }
            else {
                *parameter = strtop + 1;
                parameter++;
            }
            strtop = strend + 1;
        }
    } while ( strend );
    cli_.argv[cli_.argc][0] = '\0';
}

// ============================================================================
static void CliWriteCmdHistory( char* str ) {
    static uint8_t i = 0;

    if ( strcmp( cli_.history_cmd.cmd_buff[i], str ) ) {
        if ( cli_.history_cmd.cmd_buff[i][0] != 0 ) {
            if ( ++i >= _CLI_HISTORY_CMD_NUM ) {
                i = 0;
            }
        }
        strcpy( cli_.history_cmd.cmd_buff[i], str );
    }
    cli_.history_cmd.w_index = i;
}

// ============================================================================
static void CliReadCmdHistory( uint8_t way ) {
    way = 0;

    while ( ( cli_.history_cmd.cmd_buff[cli_.history_cmd.w_index][0] == 0 )
            && ( cli_.history_cmd.w_index != 0 ) ) {
        --cli_.history_cmd.w_index;
    }

    if ( cli_.history_cmd.cmd_buff[cli_.history_cmd.w_index][0] != 0 ) {
        for ( int i = strlen( cli_.cmd_buff_cursor ); i > 0; i-- ) {
            console.writeByte( ' ' );
        }

        for ( int i = strlen( cli_.cmd_buff ); i > 0; i-- ) {
            console.writeStr( "\b \b" );
        }

        strcpy( cli_.cmd_buff,
                &cli_.history_cmd.cmd_buff[cli_.history_cmd.w_index][0] );
        cli_.cmd_buff_cursor =
            cli_.cmd_buff
            + strlen( &cli_.history_cmd.cmd_buff[cli_.history_cmd.w_index][0] );
        cli_.cmd_buff_tail = cli_.cmd_buff_cursor;
        console.writeStr( cli_.cmd_buff );

        if ( cli_.history_cmd.w_index-- == 0 ) {
            cli_.history_cmd.w_index = _CLI_HISTORY_CMD_NUM - 1;
        }
    }
}

// ============================================================================
void CliInput( char read_char ) {
    if ( read_char == ' '
         && ( *cli_.cmd_buff_cursor == ' '
              || ( cli_.cmd_buff_cursor == cli_.cmd_buff )
              || ( *( cli_.cmd_buff_cursor - 1 ) == ' ' ) ) ) {
        return;
    }
    else {
        char str[_CLI_CMD_MAX_LEN];
        strcpy( str, cli_.cmd_buff_cursor );
        memcpy( ( cli_.cmd_buff_cursor + 1 ), str, strlen( str ) + 1 );
        *cli_.cmd_buff_cursor = read_char;
        console.writeByte( read_char );
        console.writeStr( str );
        for ( uint16_t i = strlen( str ); i > 0; i-- ) {
            console.writeByte( '\b' );
        }
        cli_.cmd_buff_cursor++;
        cli_.cmd_buff_tail++;
    }
}

// ============================================================================
void CliTabCompletion( void ) {
    console.writeStr( cli_.out_message );
    console.printf( "%s(): todo\r\n", __func__ );
}

// ============================================================================
void CliBackspace( void ) {
    char str[_CLI_CMD_MAX_LEN];

    if ( cli_.cmd_buff_cursor != cli_.cmd_buff ) {
        do {
            if ( cli_.cmd_buff_cursor == cli_.cmd_buff ) {
                console.writeStr( " " );
                cli_.cmd_buff_cursor++;
            }
            strcpy( str, cli_.cmd_buff_cursor );
            cli_.cmd_buff_cursor--;
            cli_.cmd_buff_tail--;
            memcpy( ( cli_.cmd_buff_cursor ), str, strlen( str ) + 1 );

            console.writeStr( "\b \b" );
            console.writeStr( str );
            console.writeStr( " \b" );

            for ( uint16_t i = strlen( str ); i > 0; i-- ) {
                console.writeByte( '\b' );
            }
        } while ( ( cli_.cmd_buff_cursor == cli_.cmd_buff )
                  && ( *cli_.cmd_buff_cursor == ' ' ) );
    }
}

// ============================================================================
void CliDirection( char read_char ) {
    switch ( read_char ) {
    case 'A':  // up direction key
        CliReadCmdHistory( 0 );
        break;
    case 'B':  // down direction key
        CliReadCmdHistory( 1 );
        break;
    case 'C':  // right direciton key
        if ( *cli_.cmd_buff_cursor != '\0' ) {
            console.writeByte( 0x1b );
            console.writeByte( 0x5b );
            console.writeByte( 'C' );
            cli_.cmd_buff_cursor++;
        }
        break;
    case 'D':  // left direciton key
        if ( cli_.cmd_buff_cursor != cli_.cmd_buff ) {
            console.writeByte( 0x1b );
            console.writeByte( 0x5b );
            console.writeByte( 'D' );
            cli_.cmd_buff_cursor--;
        }
        break;
    default:
        break;
    }
}

// ============================================================================
static char* CliMatchChar( char* src, char* dst ) {
    char* ptr;

    if ( *src == '#' ) {
        return dst;
    }
    if ( !dst || !src ) {
        return NULL;
    }
    if ( *src == '&' ) {
        return dst;
    }

    if ( *src == '[' ) {
        do {
            while ( *( ++src ) == ' ' ) {
                ;
            }
            ptr = dst;
            while ( *src == *ptr ) {
                src++;
                ptr++;
                if ( ( ( *ptr == ' ' ) || ( *ptr == '\0' ) )
                     && ( ( *src == ' ' ) || ( *src == '|' ) || ( *src == ']' )
                          || ( *src == '\0' ) ) ) {
                    return ( src - ( ptr - dst ) );
                }
            }
            while ( ( *src != ']' ) && ( *src != '|' ) ) {
                src++;
            }
        } while ( *src != ']' );
        return NULL;
    }

    ptr = strchr( src, ' ' );

    if ( ( ( ptr == NULL ) && ( !strcmp( src, dst ) ) )
         || ( ( ( ( ( ptr != NULL ) && ( !strncmp( src, dst, ptr - src ) ) ) )
                && ( ( *( dst + ( ptr - src ) ) == ' ' )
                     || ( *( dst + ( ptr - src ) ) == '\0' ) ) ) )
         || ( ( ptr == NULL ) && strchr( dst, '-' )
              && !strncmp( src, dst, strlen( src ) ) ) ) {
        return src;
    }
    else {
        return NULL;
    }
}

// ============================================================================
static CliCmdList_t* CliSeekCmd( char* str ) {
    uint8_t num = 0;
    char *  src, *dst;
    do {
        src = cmd_list_[num].str;
        dst = str;
        while ( ( *src == '#' ) || ( CliMatchChar( src, dst ) != NULL ) ) {
            if ( *src == '#' && ( !CliGetParam( src, 1 ) ) ) {
                return &cmd_list_[num];
            }
            dst = CliGetParam( dst, 1 );
            src = CliGetParam( src, 1 );
            while ( ( dst ) && ( *dst == '-' ) ) {
                dst = CliGetParam( dst, 1 );
            }
            if ( !src && !dst ) {
                return &cmd_list_[num];
            }
        }
    } while ( cmd_list_[++num].str );
    return NULL;
}

// ============================================================================
void CliProcessCmd( char* str ) {
    HAL_StatusTypeDef ret = HAL_OK;
    CliCmdList_t*     cmd;
    char*             parameter[10] = { NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL, NULL };

    char** ptr = parameter;

    CliWriteCmdHistory( str );
    CliFormatCmd( str, ptr );
    cmd = CliSeekCmd( str );

    if ( cmd != NULL ) {
        while ( cmd != NULL ) {
            ret = cmd->p( cli_.argc, cli_.argv );
            if ( strstr( str, "-t" ) == NULL ) {
                break;
            }
            if ( console.read( 0 ) != 0 ) {
                break;
            }
        }
        if ( HAL_OK != ret ) {
            console.writeStr( cli_.out_message );
            console.writeStr(
                "\033[0;31mcommand execution failed!\033[0m\r\n" );
        }
    }
    else {
        console.writeStr( cli_.out_message );
        console.writeStr( "\033[0;31mcommand not recognized!\033[0m "
                          "try \033[0;32mhelp\033[0m.\r\n" );
    }
}

// ============================================================================
HAL_StatusTypeDef CliShowCmd( void ) {

    console.writeStr( "\r\n" );
    CONSOLE_PRINTF_SEG;
    console.writeStr( "console registered commands:" );
    for ( uint8_t i = 0; i < _CLI_CMD_MAX_NUM; i++ ) {
        if ( cmd_list_[i].str ) {
            console.writeStr( "\r\n  " );
            console.writeStr( cmd_list_[i].str );
        }
    }
    console.writeStr( "\r\n" );
    CONSOLE_PRINTF_SEG;
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliReset( void ) {
    NVIC_SystemReset();
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliLogSetLevel( int argc, char** argv ) {
    if ( argc <= 1 ) {
        return HAL_ERROR;
    }
    if ( strcmp( argv[1], "view" ) == 0 ) {
        console.printk( LOG_INFO, "\r\n LOG_INFO" );
        console.printk( LOG_WARNING, "\r\n LOG_WARNING" );
        console.printk( LOG_CRIT, "\r\n LOG_CRIT" );
    }
    else if ( strcmp( argv[1], "lower" ) == 0 ) {
        if ( console.level > 0 ) {
            console.level -= 1;
        }
        console.cli.setLevel( console.level );
        console.printk( 0, "\r\n log level set to %d", console.level );
    }
    else if ( strcmp( argv[1], "higher" ) == 0 ) {
        if ( console.level < LOG_INFO ) {
            console.level += 1;
        }
        console.cli.setLevel( console.level );
        console.printk( 0, "\r\n log level set to %d", console.level );
    }
    else {
        LogLevel_e level = atoi( argv[1] );
        if ( level <= LOG_INFO ) {
            console.cli.setLevel( level );
            console.printk( 0, "\r\n log level set to %d", level );
        }
        else {
            console.printk( 0, "\r\n error, 0 <= level <= 2" );
        }
    }
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliCheckFirmware( void ) {

    console.writeStr( "\r\n" );
    CONSOLE_PRINTF_SEG;
    char* ptr;
    ( void )ptr;
    // --------------------------------
#if defined( FIRMWARE )
    console.printk( 0, " firmware   :" WHT " %s\r\n" NOC, FIRMWARE );
#endif

    // --------------------------------
#if defined( PRJ_GIT_VER ) && defined( PRJ_GIT_CMT )
    ptr = ( char* )&PRJ_GIT_VER + strlen( PRJ_GIT_VER ) - 5;
    if ( strcmp( ptr, "dirty" ) == 0 ) {
        console.printk( 0, " version    :" RED " %s" NOC " (%s)\r\n",
                        PRJ_GIT_VER, PRJ_GIT_CMT );
    }
    else {
        console.printk( 0, " version    :" WHT " %s" NOC " (%s)\r\n",
                        PRJ_GIT_VER, PRJ_GIT_CMT );
    }
#endif

    // --------------------------------
#if defined( PRJ_GIT_BRH )
    console.printk( 0, " branch     :" WHT " %s\r\n" NOC, PRJ_GIT_BRH );
#endif

    // --------------------------------
#if defined( LIB_GIT_VER ) && defined( LIB_GIT_CMT )
    ptr = ( char* )&LIB_GIT_VER + strlen( LIB_GIT_VER ) - 5;
    if ( strcmp( ptr, "dirty" ) == 0 ) {
        console.printk( 0, " lib version:" RED " %s" NOC " (%s)\r\n",
                        LIB_GIT_VER, LIB_GIT_CMT );
    }
    else {
        console.printk( 0, " lib version:" WHT " %s" NOC " (%s)\r\n",
                        LIB_GIT_VER, LIB_GIT_CMT );
    }
#endif

    // --------------------------------
#if defined( LIB_GIT_BRH )
    console.printk( 0, " lib branch :" WHT " %s\r\n" NOC, LIB_GIT_BRH );
#endif

    // --------------------------------
#if defined( MAKE_TYPE )
    console.printk( 0, " make type  :" WHT " %s\r\n" NOC, MAKE_TYPE );
#endif

    // --------------------------------
    console.printk( 0, " make time  :" WHT " %s, %s\r\n" NOC, __TIME__,
                    __DATE__ );

    CONSOLE_PRINTF_SEG;
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliShowScheduler( int argc, char** argv ) {
    if ( argc <= 1 ) {
        return HAL_ERROR;
    }
    if ( strcmp( argv[1], "view" ) == 0 ) {
        console.printk( 0, "\r\n" );
#if defined( _STIME_USE_SCHEDULER )
        stime.scheduler.show();
#endif
    }
    else {
        console.printk( 0, "\r\n" );
        console.printk( 0, "argument not recognized.\r\n" );
        return HAL_ERROR;
    }
    return HAL_OK;
}
