#include "config.h"

#include "console-cli.h"
#include "math13rd.h"
#include <stdlib.h>
#include <string.h>

#if defined(CONSOLE_IS_USED)
Cli_t cli_;
CliCmd_t cmd_[_CLI_CMD_MAX_NUM];

// ============================================================================
void CliDeInit() {
    cli_.cmd_buff_tail = cli_.cmd_buff;
    cli_.cmd_buff_cursor = cli_.cmd_buff;
    cli_.cmd_buff[0] = '\0';
}

// ============================================================================
static char* CliGetParam(char* str, uint8_t num) {
    char* dst = str;
    uint8_t i = 0;
    while (*dst == ' ') {
        dst++;
    }
    if (num == 0 && *dst) {
        return dst;
    }
    while (*dst) {
        if (*dst == '[') {
            dst = strchr(dst, ']') + 1;
            if (dst == NULL) {
                console.printk(0, "\r\ninstruction standard error!\r\n");
                return NULL;
            }
        }
        else if (*dst++ == ' ') {
            while (*dst == ' ')
                dst++;
            i++;
            if (i == num && *dst) {
                return dst;
            }
        }
    }
    return NULL;
}

// ============================================================================
static void CliFormatCmd(char* strtop, char** parameter) {
    char* strend;

    cli_.argc = 0;
    cli_.argv[0] = cli_.argv_buff;

    do {
        strend = strchr(strtop, ' ');
        if (strend == NULL) {
            if (strncmp(strtop, "-all", 4)) {
                strcpy(cli_.argv[cli_.argc], strtop);
                *(cli_.argv[cli_.argc++] + strlen(strtop)) = '\0';
                cli_.argv[cli_.argc] =
                    cli_.argv[cli_.argc - 1] + strlen(strtop) + 1;
            }
            else {
                *parameter = strtop + 1;
                parameter++;
            }
        }
        else {
            if (strncmp(strtop, "-all", 4)) {
                strncpy(cli_.argv[cli_.argc], strtop,
                        (size_t)(strend - strtop));
                *(cli_.argv[cli_.argc++] + (strend - strtop)) = '\0';
                cli_.argv[cli_.argc] =
                    cli_.argv[cli_.argc - 1] + (strend - strtop) + 1;
            }
            else {
                *parameter = strtop + 1;
                parameter++;
            }
            strtop = strend + 1;
        }
    } while (strend);
    cli_.argv[cli_.argc][0] = '\0';
}

// ============================================================================
static void CliWriteCmdHistory(char* str) {
    static uint8_t i = 0;
    if (strcmp(cli_.history.buff[i], str)) {
        if (cli_.history.buff[i][0] != 0) {
            if (++i >= _CLI_HISTORY_CMD_NUM) {
                i = 0;
            }
        }
        strcpy(cli_.history.buff[i], str);
    }
    cli_.history.index = i;
}

// ============================================================================
static void CliShowAllCmdHistory(void) {
    for (int i = 0; i < _CLI_HISTORY_CMD_NUM; i++) {
        console.printk(0, "%d: %s\r\n", i, cli_.history.buff[i]);
    }
    console.printk(0, "index = %d\r\n", cli_.history.index);
}

// ============================================================================
static void CliReadCmdHistory(uint8_t way) {
    way = 0;

    while ((cli_.history.buff[cli_.history.index][0] == 0)
           && (cli_.history.index != 0)) {
        --cli_.history.index;
    }

    if (cli_.history.buff[cli_.history.index][0] != 0) {
        for (size_t i = strlen(cli_.cmd_buff_cursor); i > 0; i--) {
            console.write.byte(' ');
        }

        for (size_t i = strlen(cli_.cmd_buff); i > 0; i--) {
            console.write.str("\b \b");
        }

        strcpy(cli_.cmd_buff, &cli_.history.buff[cli_.history.index][0]);
        cli_.cmd_buff_cursor =
            cli_.cmd_buff + strlen(&cli_.history.buff[cli_.history.index][0]);
        cli_.cmd_buff_tail = cli_.cmd_buff_cursor;
        console.write.str(cli_.cmd_buff);

        if (cli_.history.index-- == 0) {
            cli_.history.index = _CLI_HISTORY_CMD_NUM - 1;
        }
    }
}

// ============================================================================
void CliInput(char read_char) {
    if (read_char == ' '
        && (*cli_.cmd_buff_cursor == ' '
            || (cli_.cmd_buff_cursor == cli_.cmd_buff)
            || (*(cli_.cmd_buff_cursor - 1) == ' '))) {
        return;
    }
    else {
        char str[_CLI_CMD_MAX_LEN];
        strcpy(str, cli_.cmd_buff_cursor);
        memcpy((cli_.cmd_buff_cursor + 1), str, strlen(str) + 1);
        *cli_.cmd_buff_cursor = read_char;
        console.write.byte(read_char);
        console.write.str(str);
        for (size_t i = strlen(str); i > 0; i--) {
            console.write.byte('\b');
        }
        cli_.cmd_buff_cursor++;
        cli_.cmd_buff_tail++;
    }
}

// ============================================================================
void CliTabCompletion(char read_char) {
    (void)read_char;
    console.write.str(cli_.out_message);
    console.printk(0, "%s(): todo\r\n", __func__);
}

// ============================================================================
void CliBackspace(void) {
    char str[_CLI_CMD_MAX_LEN];

    if (cli_.cmd_buff_cursor != cli_.cmd_buff) {
        do {
            if (cli_.cmd_buff_cursor == cli_.cmd_buff) {
                console.write.str(" ");
                cli_.cmd_buff_cursor++;
            }
            strcpy(str, cli_.cmd_buff_cursor);
            cli_.cmd_buff_cursor--;
            cli_.cmd_buff_tail--;
            memcpy((cli_.cmd_buff_cursor), str, strlen(str) + 1);

            console.write.str("\b \b");
            console.write.str(str);
            console.write.str(" \b");

            for (size_t i = strlen(str); i > 0; i--) {
                console.write.byte('\b');
            }
        } while ((cli_.cmd_buff_cursor == cli_.cmd_buff)
                 && (*cli_.cmd_buff_cursor == ' '));
    }
}

// ============================================================================
void CliDirection(char read_char) {
    switch (read_char) {
    case 'A':  // up direction key
        // console.printk( 0, "up" );
        CliReadCmdHistory(0);
        break;
    case 'B':  // down direction key
        // console.printk( 0, "down" );
        CliReadCmdHistory(1);
        break;
    case 'C':  // right direciton key
        // console.printk( 0, "right" );
        if (*cli_.cmd_buff_cursor != '\0') {
            console.write.byte(0x1b);
            console.write.byte(0x5b);
            console.write.byte('C');
            cli_.cmd_buff_cursor++;
        }
        break;
    case 'D':  // left direciton key
        // console.printk( 0, "left" );
        if (cli_.cmd_buff_cursor != cli_.cmd_buff) {
            console.write.byte(0x1b);
            console.write.byte(0x5b);
            console.write.byte('D');
            cli_.cmd_buff_cursor--;
        }
        break;
    default:
        break;
    }
}

// ============================================================================
static char* CliMatchChar(char* src, char* dst) {
    char* ptr;

    if (*src == '#') {
        return dst;
    }
    if (!dst || !src) {
        return NULL;
    }
    if (*src == '&') {
        return dst;
    }

    if (*src == '[') {
        do {
            while (*(++src) == ' ') {
                ;
            }
            ptr = dst;
            while (*src == *ptr) {
                src++;
                ptr++;
                if (((*ptr == ' ') || (*ptr == '\0'))
                    && ((*src == ' ') || (*src == '|') || (*src == ']')
                        || (*src == '\0'))) {
                    return (src - (ptr - dst));
                }
            }
            while ((*src != ']') && (*src != '|')) {
                src++;
            }
        } while (*src != ']');
        return NULL;
    }

    ptr = strchr(src, ' ');

    if (((ptr == NULL) && (!strcmp(src, dst)))
        || (((((ptr != NULL) && (!strncmp(src, dst, (size_t)(ptr - src)))))
             && ((*(dst + (ptr - src)) == ' ')
                 || (*(dst + (ptr - src)) == '\0'))))
        || ((ptr == NULL) && strchr(dst, '-')
            && !strncmp(src, dst, strlen(src)))) {
        return src;
    }
    else {
        return NULL;
    }
}

// ============================================================================
static CliCmd_t* CliSeekCmd(char* str) {
    uint8_t num = 0;
    char *src, *dst;
    do {
        src = cmd_[num].str;
        dst = str;
        while ((*src == '#') || (CliMatchChar(src, dst) != NULL)) {
            if (*src == '#' && (!CliGetParam(src, 1))) {
                return &cmd_[num];
            }
            dst = CliGetParam(dst, 1);
            src = CliGetParam(src, 1);
            while ((dst) && (*dst == '-')) {
                dst = CliGetParam(dst, 1);
            }
            if (!src && !dst) {
                return &cmd_[num];
            }
        }
    } while (cmd_[++num].str);
    return NULL;
}

// ============================================================================
void CliProcessCmd(char* str) {
    HAL_StatusTypeDef ret = HAL_OK;
    CliCmd_t* cmd;

    char* parameter[10] = { NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL, NULL };

    char** ptr = parameter;

    CliWriteCmdHistory(str);
    // CliShowAllCmdHistory();
    (void)CliShowAllCmdHistory;
    CliFormatCmd(str, ptr);
    cmd = CliSeekCmd(str);
    if (cmd != NULL) {
        while (cmd != NULL) {
            ret = cmd->p(cli_.argc, cli_.argv);
            if (strstr(str, "-t") == NULL) {
                break;
            }
            if (console.read(0) != 0) {
                break;
            }
        }
        if (HAL_OK != ret) {
            console.printk(0, RED "\r\ncommand execution failed!" NOC);
        }
    }
    else {
        console.printk(0, RED "\r\ncommand not recognized!" NOC);
        CliShowCmd();
    }
}

// ============================================================================
HAL_StatusTypeDef CliShowCmd(void) {

    console.write.str("\r\n");
    CONSOLE_PRINTF_SEG;
    console.write.str("console registered commands:");
    for (uint8_t i = 0; i < _CLI_CMD_MAX_NUM; i++) {
        if (cmd_[i].str) {
            console.write.str("\r\n  ");
            console.write.str(cmd_[i].str);
        }
    }
    console.write.str("\r\n");
    CONSOLE_PRINTF_SEG;
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliLogSetLevel(int argc, char** argv) {
    if (argc <= 1) {
        return HAL_ERROR;
    }
    if (strcmp(argv[1], "help") == 0) {
        console.printk(0, "\r\n");
        console.printk(0, "log\r\n");
        console.printk(0, "   view: see available log levels\r\n");
        console.printk(0, "   down: lower the log level\r\n");
        console.printk(0, "   rise: rise the log level\r\n");
        console.printk(0, "    [x]: set log level to x");
        return HAL_OK;
    }
    if (strcmp(argv[1], "view") == 0) {
        console.printk(0, "\r\n   LOG_INFO = 2");
        console.printk(0, "\r\n   LOG_WARNING = 1");
        console.printk(0, "\r\n   LOG_CRIT = 0");
        return HAL_OK;
    }
    if (strcmp(argv[1], "down") == 0) {
        if (console.level > 0) {
            console.level -= 1;
        }
        console.cli.setLevel(console.level);
        console.printk(0, "\r\n log level set to %d", console.level);
        return HAL_OK;
    }
    if (strcmp(argv[1], "rise") == 0) {
        if (console.level < LOG_INFO) {
            console.level += 1;
        }
        console.cli.setLevel(console.level);
        console.printk(0, "\r\n log level set to %d", console.level);
        return HAL_OK;
    }
    else {
        LogLevel_e level = atoi(argv[1]);
        if (level <= LOG_INFO) {
            console.cli.setLevel(level);
            console.printk(0, "\r\n log level set to %d", level);
        }
        else {
            console.printk(0, "\r\n error, 0 <= level <= 2");
        }
    }
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliShowFirmware(void) {

    console.write.str("\r\n");
    CONSOLE_PRINTF_SEG;
    char* ptr;
    (void)ptr;
    // --------------------------------
#if defined(FIRMWARE)
    console.printk(0, "    firmware:" WHT " %s\r\n" NOC, FIRMWARE);
#endif

    // --------------------------------
#if defined(PRJ_GIT_VER) && defined(PRJ_GIT_CMT)
    ptr = (char*)&PRJ_GIT_VER + strlen(PRJ_GIT_VER) - 5;
    if (strcmp(ptr, "dirty") == 0) {
        console.printk(0, "     version:" RED " %s" NOC " (%s)\r\n",
                       PRJ_GIT_VER, PRJ_GIT_CMT);
    }
    else {
        console.printk(0, "     version:" WHT " %s" NOC " (%s)\r\n",
                       PRJ_GIT_VER, PRJ_GIT_CMT);
    }
#endif

    // --------------------------------
#if defined(PRJ_GIT_BRH)
    console.printk(0, "      branch:" WHT " %s\r\n" NOC, PRJ_GIT_BRH);
#endif

    // --------------------------------
#if defined(LIB_GIT_VER) && defined(LIB_GIT_CMT)
    ptr = (char*)&LIB_GIT_VER + strlen(LIB_GIT_VER) - 5;
    if (strcmp(ptr, "dirty") == 0) {
        console.printk(0, " lib version:" RED " %s" NOC " (%s)\r\n",
                       LIB_GIT_VER, LIB_GIT_CMT);
    }
    else {
        console.printk(0, " lib version:" WHT " %s" NOC " (%s)\r\n",
                       LIB_GIT_VER, LIB_GIT_CMT);
    }
#endif

    // --------------------------------
#if defined(LIB_GIT_BRH)
    console.printk(0, "  lib branch:" WHT " %s\r\n" NOC, LIB_GIT_BRH);
#endif

    // --------------------------------
#if defined(MAKE_TYPE)
    console.printk(0, "   make type:" WHT " %s\r\n" NOC, MAKE_TYPE);
#endif

    // --------------------------------
    console.printk(0, "   make time:" WHT " %s, %s\r\n" NOC, __TIME__,
                   __DATE__);

    CONSOLE_PRINTF_SEG;
    return HAL_OK;
}

// ============================================================================
HAL_StatusTypeDef CliSystem(int argc, char** argv) {
    if (argc <= 1) {
        return HAL_ERROR;
    }
    if (strcmp(argv[1], "firmware") == 0) {
        CliShowFirmware();
        return HAL_OK;
    }
    if ((strcmp(argv[1], "reset") == 0) || (strcmp(argv[1], "reboot") == 0)) {
        NVIC_SystemReset();
        return HAL_OK;
    }
    if (strcmp(argv[1], "flash") == 0) {
        console.printk(0, "\r\ntodo\r\n");
        return HAL_OK;
    }
    if (strcmp(argv[1], "help") == 0) {
        console.printk(0, "\r\n");
        console.printk(0, "system\r\n");
        console.printk(0, "        firmware: check firmware information\r\n");
        console.printk(0, "    reset/reboot: reboot the system\r\n");
        console.printk(
            0, "           flash: flash new firmware (need hardware support)");
        return HAL_OK;
    }
    console.printk(0, "\r\n");
    console.printk(0, "argument not recognized.\r\n");
    return HAL_ERROR;
}

// ============================================================================
HAL_StatusTypeDef CliShowScheduler(int argc, char** argv) {
    if (argc <= 1) {
        return HAL_ERROR;
    }
    if (strcmp(argv[1], "view") == 0) {
        console.printk(0, "\r\n");
#if defined(_STIME_USE_SCHEDULER)
        stime.scheduler.show();
#endif
        return HAL_OK;
    }
    if (strcmp(argv[1], "help") == 0) {
        console.printk(0, "\r\n");
        console.printk(0, "scheduler\r\n");
        console.printk(0, "    view: show scheduler detail");
        return HAL_OK;
    }
    console.printk(0, "\r\n");
    console.printk(0, "argument not recognized.\r\n");
    return HAL_ERROR;
}

// ============================================================================
#if defined(_STIME_USE_SCHEDULER)
// this function works only when the scheduler is used
HAL_StatusTypeDef CliSuspend(int argc, char** argv) {
    // FIXME: seems like my CLI does not support different number of arguments
    if (argc <= 1) {
        return HAL_ERROR;
    }
#if defined(_USE_ID)
    if (argc <= 2) {
        return HAL_ERROR;
    }
#endif  // _USE_ID
    if (strcmp(argv[1], "help") == 0) {
        console.printk(0, "\r\n");
        console.printk(0, "cli-suspend [x] [id]\r\n");
        console.printk(0, "    [x]: suspend the cli for x seconds\r\n");
        console.printk(0, "   [id]: exclude system ID (id)");
        return HAL_OK;
    }
    if (strspn(argv[1], "0123456789\n") != strlen(argv[1]))
        return HAL_ERROR;

#if defined(_USE_ID)
    // if "id" contains other characters, do not suspend
    if (strspn(argv[2], "0123456789\n") != strlen(argv[2]))
        return HAL_OK;
    uint32_t id = (uint32_t)atoi(argv[2]);
    // exclude the ID
    if (id == sid.get()) {
        return HAL_OK;
    }
#endif  // _USE_ID
    uint32_t seconds = (uint32_t)atoi(argv[1]);
    stime.scheduler.cliSuspend(seconds);
    console.rx.setStatus(false);
    return HAL_OK;
}
#endif  // _STIME_USE_SCHEDULER

#endif  // CONSOLE_IS_USED
