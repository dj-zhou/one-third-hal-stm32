#include "stime-scheduler.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
#if defined(STIME_IS_USED)
// 32-bit tick overflow at different frequency
// _STIME_4K_TICK   2^32/4/1000 =  12.427 days ( 1073741.824 s)
// _STIME_2K_TICK   2^32/2/1000 =  24.854 days
// _STIME_1K_TICK   2^32/1000   =  49.708 days
// _STIME_500_TICK  2^32/500    =  99.416 days
// _STIME_400_TICK  2^32/400    = 124.270 days
// _STIME_200_TICK  2^32/200    = 248.540 days
// so just let it overflow at 1s
static volatile uint32_t tick_;    // as 1/4ms, 1/2ms, 1ms, 2ms or 5ms, up to 1s
static volatile uint32_t second_;  // use this to support 136 years

// if cli_suspend_second_ > second_, cli will not run:
volatile uint32_t cli_suspend_second_ = 0;

// ============================================================================
// constant values used accros the file -------------
// clang-format off
#if defined(_STIME_USE_SYSTICK)
    #if defined(_STIME_4K_TICK)
        #define SYSTICK_1S_OVERFLOW     4000
        #define SYSTICK_MS_SCALE         250
    #elif defined(_STIME_2K_TICK)
        #define SYSTICK_1S_OVERFLOW     2000
        #define SYSTICK_MS_SCALE         500
    #elif defined(_STIME_1K_TICK)
        #define SYSTICK_1S_OVERFLOW     1000
        #define SYSTICK_MS_SCALE        1000
    #elif defined(_STIME_500_TICK)
        #define SYSTICK_1S_OVERFLOW      500
        #define SYSTICK_MS_SCALE        2000
    #elif defined(_STIME_400_TICK)
        #define SYSTICK_1S_OVERFLOW      400
        #define SYSTICK_MS_SCALE        2500
    #elif defined(_STIME_200_TICK)
        #define SYSTICK_1S_OVERFLOW      200
        #define SYSTICK_MS_SCALE        5000
    #endif
#endif
// STM32F030x8 uses 48 MHz system clock
#if defined(STM32F030x8)
    #define US_TICK   6
    #if defined(_STIME_USE_SYSTICK)
        #if defined(_STIME_4K_TICK)
            #define SYSTICK_RELOAD_VALUE  1500
        #elif defined(_STIME_2K_TICK)
            #define SYSTICK_RELOAD_VALUE  3000
        #elif defined(_STIME_1K_TICK)
            #define SYSTICK_RELOAD_VALUE  6000
        #elif defined(_STIME_500_TICK)
            #define SYSTICK_RELOAD_VALUE 12000
        #elif defined(_STIME_400_TICK)
            #define SYSTICK_RELOAD_VALUE 15000
        #elif defined(_STIME_200_TICK)
            #define SYSTICK_RELOAD_VALUE 30000
        #endif
    #endif
// STM32F103xB uses 72 MHz system clock
// STM32F107xC uses 72 MHz system clock
// STM32F303xE uses 72 MHz system clock
#elif defined(STM32F103xB) || defined(STM32F107xC) || defined(STM32F303xE)
    #define US_TICK   9
    #if defined(_STIME_USE_SYSTICK)
        #if defined(_STIME_4K_TICK)
            #define SYSTICK_RELOAD_VALUE  2250
        #elif defined(_STIME_2K_TICK)
            #define SYSTICK_RELOAD_VALUE  4500
        #elif defined(_STIME_1K_TICK)
            #define SYSTICK_RELOAD_VALUE  9000
        #elif defined(_STIME_500_TICK)
            #define SYSTICK_RELOAD_VALUE 18000
        #elif defined(_STIME_400_TICK)
            #define SYSTICK_RELOAD_VALUE 22500
        #elif defined(_STIME_200_TICK)
            #define SYSTICK_RELOAD_VALUE 45000
        #endif
    #endif
// STM32F407 uses 168 MHz system clock
#elif defined(STM32F407xx)
    #define US_TICK   21
    #if defined(_STIME_USE_SYSTICK)
        #if defined(_STIME_4K_TICK)
            #define SYSTICK_RELOAD_VALUE   5250
        #elif defined(_STIME_2K_TICK)
            #define SYSTICK_RELOAD_VALUE  10500
        #elif defined(_STIME_1K_TICK)
            #define SYSTICK_RELOAD_VALUE  21000
        #elif defined(_STIME_500_TICK)
            #define SYSTICK_RELOAD_VALUE  42000
        #elif defined(_STIME_400_TICK)
            #define SYSTICK_RELOAD_VALUE  52500
        #elif defined(_STIME_200_TICK)
            #define SYSTICK_RELOAD_VALUE 105000
        #endif
    #endif
// STM32F427 uses 180 MHz system clock, why not use 176Mhz?
#elif defined(STM32F427xx)
    #define US_TICK   22 // 22.5 is the right number ... to fix
    #if defined(_STIME_USE_SYSTICK)
        #if defined(_STIME_4K_TICK)
            #define SYSTICK_RELOAD_VALUE   5625
        #elif defined(_STIME_2K_TICK)
            #define SYSTICK_RELOAD_VALUE  11250
        #elif defined(_STIME_1K_TICK)
            #define SYSTICK_RELOAD_VALUE  22500
        #elif defined(_STIME_500_TICK)
            #define SYSTICK_RELOAD_VALUE  45000
        #elif defined(_STIME_400_TICK)
            #define SYSTICK_RELOAD_VALUE  56250
        #elif defined(_STIME_200_TICK)
            #define SYSTICK_RELOAD_VALUE 112500
        #endif
    #endif
// STM32F746 uses 216 MHz system clock
// STM32F767 uses 216 MHz system clock
#elif defined(STM32F746xx) || defined(STM32F767xx)
    #define US_TICK   27
    #if defined(_STIME_USE_SYSTICK)
        #if defined(_STIME_4K_TICK)
            #define SYSTICK_RELOAD_VALUE   6750
        #elif defined(_STIME_2K_TICK)
            #define SYSTICK_RELOAD_VALUE  13500
        #elif defined(_STIME_1K_TICK)
            #define SYSTICK_RELOAD_VALUE  27000
        #elif defined(_STIME_500_TICK)
            #define SYSTICK_RELOAD_VALUE  54000
        #elif defined(_STIME_400_TICK)
            #define SYSTICK_RELOAD_VALUE  67500
        #elif defined(_STIME_200_TICK)
            #define SYSTICK_RELOAD_VALUE 135000
        #endif
    #endif
// #elif ...
#endif
// clang-format on

// ============================================================================
// SysTick is a 24-bit count down counter
// if FreeRTOS is used, this functions may be overwritten
static void InitSysTick(void) {
    tick_   = 0;
    second_ = 0;
    HAL_SYSTICK_Config(SYSTICK_RELOAD_VALUE - 1U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

// ----------------------------------------------------------------------------
static Stime_t GetSysTickTime(void) {
    // disable the SysTick, or use HAL_SuspendTick()
    CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
    uint32_t val = SysTick->VAL;
    Stime_t  st;
    st.s  = second_;
    st.us = tick_ * SYSTICK_MS_SCALE + (SYSTICK_RELOAD_VALUE - val) / US_TICK;
    // enable the SysTick, or use HAL_ResumeTick()
    SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
    return st;
}

// ----------------------------------------------------------------------------
static Stime_t tic_, toc_;
static void    stimeTic(void) {
    tic_ = GetSysTickTime();
}

// ----------------------------------------------------------------------------
static void stimeToc(char* mu, char* message) {
    toc_ = GetSysTickTime();
    long time;
    if (strcmp(mu, "ms") == 0) {
        time = (toc_.s - tic_.s) * 1000 + (toc_.us - tic_.us) / 1000;
        console.printf("%s consumes %ld ms.\r\n", message, time);
        return;
    }
    if (strcmp(mu, "us") == 0) {
        time = (toc_.s - tic_.s) * 1000000 + (toc_.us - tic_.us);
        console.printf("%s consumes %ld us.\r\n", message, time);
        return;
    }
}

// ----------------------------------------------------------------------------
// accurate now. but sometimes loose one tick?
static void DelayUs(uint32_t us) {
    if (us <= 0) {
        return;
    }
    volatile Stime_t cur_time = GetSysTickTime();
    uint32_t         cur_us   = cur_time.s * 1000000 + ( uint32_t )cur_time.us;
    uint32_t         final_us = cur_us + ( uint32_t )us;
    do {
        cur_time.s  = second_;
        cur_time.us = tick_ * SYSTICK_MS_SCALE
                      + (SYSTICK_RELOAD_VALUE - SysTick->VAL) / US_TICK;
        cur_us = cur_time.s * 1000000 + ( uint32_t )cur_time.us;

        for (uint8_t i = 0; i < 2; i++) {
            __asm("MOV R0,R0");  // make sure it is not optimized
        }
    } while (cur_us <= final_us);
}

// ----------------------------------------------------------------------------
static void DelayMs(uint32_t ms) {
    if (ms <= 0) {
        return;
    }
    return DelayUs(ms * 1000);
}

#if defined(_STIME_USE_SCHEDULER)
TaskNode_t     node_[_STIME_TASK_MAX_NUM];
static uint8_t task_num_ = 0;

// ----------------------------------------------------------------------------
static void SchedulerConfig(void) {
    // data initialization !! ------------------------
    for (uint16_t i = 0; i < _STIME_TASK_MAX_NUM; i++) {
        node_[i]._this.run   = 0;
        node_[i]._this.time  = UINT16_MAX;
        node_[i]._this.ticks = UINT16_MAX;
        memset(node_[i]._this.name, 0, _STIME_TASK_NAME_LEN);
        node_[i]._this.handle = NULL;
        node_[i]._next        = NULL;
    }
}

// ----------------------------------------------------------------------------
static void TasksMark(void) {
    for (uint8_t i = 0; i < task_num_; i++) {
        if (node_[i]._this.time) {
            node_[i]._this.time--;
            if (node_[i]._this.time == 0) {
                node_[i]._this.time = node_[i]._this.ticks;
                // mark it as run, then then system will run it
                node_[i]._this.run = 1;
            }
        }
    }
}

// ----------------------------------------------------------------------------
static uint32_t IntervalToTicks(uint32_t interval_ms) {
    uint32_t ticks;
#if defined(_STIME_4K_TICK)
    if (interval_ms == _1_TICK) {
        ticks = 1;
    }
    else if (interval_ms == _2_TICK) {
        ticks = 2;
    }
    else if (interval_ms == _3_TICK) {
        ticks = 3;
    }
    else {
        ticks = interval_ms << 2;
    }
#endif

#if defined(_STIME_2K_TICK)
    if (interval_ms == _1_TICK) {
        ticks = 1;
    }
    else {
        ticks = interval_ms << 1;
    }
#endif

#if defined(_STIME_1K_TICK)
    ticks = interval_ms;
#endif

#if defined(_STIME_500_TICK)
    ticks = interval_ms / 2;
#endif

#if defined(_STIME_400_TICK)
    ticks = (uint32_t)(interval_ms / 2.5);
#endif

#if defined(_STIME_200_TICK)
    ticks = interval_ms / 5;
#endif

    return ticks;
}

// ----------------------------------------------------------------------------
static void SchedulerAttachTask(uint32_t interval_ms, uint32_t time_init,
                                TaskHandle task_handle, const char* task_name) {
    if (time_init == 0) {
        console.error("%s: wrong argument, time_init cannot be 0!\r\n",
                      __func__);
    }
    if (task_num_ > _STIME_TASK_MAX_NUM) {
        console.error("%s: too many tasks been registered!\r\n,__func__",
                      __func__);
    }
    uint32_t ticks = IntervalToTicks(interval_ms);
    uint8_t  len   = strlen(task_name) < _STIME_TASK_NAME_LEN
                      ? strlen(task_name)
                      : _STIME_TASK_NAME_LEN;
    if (task_num_ == 0) {
        node_[0]._this.run    = 0;
        node_[0]._this.time   = time_init;
        node_[0]._this.ticks  = ticks;
        node_[0]._this.handle = task_handle;
        snprintf(node_[0]._this.name, len + 1, task_name);

        node_[0]._next = NULL;
    }
    else {
        uint8_t iter             = task_num_;
        node_[iter - 1]._next    = &node_[iter];
        node_[iter]._this.run    = 0;
        node_[iter]._this.time   = time_init;
        node_[iter]._this.ticks  = ticks;
        node_[iter]._this.handle = task_handle;
        snprintf(node_[iter]._this.name, len + 1, task_name);
    }
    task_num_++;
}

// ----------------------------------------------------------------------------
static void SchedulerProcess(void) {
    for (uint8_t i = 0; i < task_num_; i++) {
        if (node_[i]._this.run) {
            node_[i]._this.handle();
            node_[i]._this.run = 0;
        }
    }
}

// ----------------------------------------------------------------------------
static void SchedulerShowTasks(void) {

    CONSOLE_PRINTF_SEG;
    console.printk(0, " Stime Task Scheduler (");
#if defined(_STIME_4K_TICK)
    console.printk(0, "4K Hz tick)");
#elif defined(_STIME_2K_TICK)
    console.printk(0, "2K Hz tick)");
#elif defined(_STIME_1K_TICK)
    console.printk(0, "1K Hz tick)");
#elif defined(_STIME_500_TICK)
    console.printk(0, "500 Hz tick)");
#elif defined(_STIME_400_TICK)
    console.printk(0, "400 Hz tick)");
#elif defined(_STIME_200_TICK)
    console.printk(0, "200 Hz tick)");
#endif
    if (task_num_ == 0) {
        console.printk(0, "  |  no task\r\n");
        CONSOLE_PRINTF_SEG;
        return;
    }
    if (task_num_ == 1) {
        console.printk(0, "  |  1 task\r\n");
    }
    else {
        console.printk(0, " | %2d tasks\r\n", task_num_);
    }
    for (uint8_t i = 0; i < task_num_; i++) {
        console.printk(0, " %2d (%d): %s\r\n", i + 1, node_[i]._this.ticks,
                       node_[i]._this.name);
    }
    CONSOLE_PRINTF_SEG;
}

// ----------------------------------------------------------------------------
void SchedulerRun(void) {
    while (true) {
        if (cli_suspend_second_ <= second_) {
            if (!console.getRxStatus()) {
                console.setRxStatus(true);
            }
            console.cli.process();
        }
        SchedulerProcess();
    }
}

// ----------------------------------------------------------------------------
void SchedulerCliSuspendTime(uint32_t seconds) {
    Stime_t cur_time    = GetSysTickTime();
    cli_suspend_second_ = cur_time.s + seconds;
}
#endif  // _STIME_USE_SCHEDULER

// ----------------------------------------------------------------------------
// when the interrupt is called, the value in SysTick->LOAD is reloaded to
// SysTick->VAL
void SysTick_Handler(void) {
    tick_++;
    if (tick_ >= SYSTICK_1S_OVERFLOW) {
        tick_ = 0;
        second_++;
        // utils.pin.toggle( GPIOD, 4 );  // test only
    }
#if defined(_STIME_USE_SCHEDULER)
    TasksMark();
#endif
}

#endif  // STIME_IS_USED

// ============================================================================
#if defined(STIME_IS_USED)
// clang-format off
StimeApi_t stime = {
#if defined( _STIME_USE_SYSTICK )
    .config   = InitSysTick   ,
    .getTime  = GetSysTickTime,
    .tic      = stimeTic      ,
    .toc      = stimeToc      ,
    .delay.us = DelayUs       ,
    .delay.ms = DelayMs       ,
#else
    #error StimeApi_t stime: not implemented.
#endif

#if defined( _STIME_USE_SCHEDULER )
    .scheduler.config     = SchedulerConfig         ,
    .scheduler.attach     = SchedulerAttachTask     ,
    .scheduler.show       = SchedulerShowTasks      ,
    .scheduler.run        = SchedulerRun            ,
    .scheduler.cliSuspend = SchedulerCliSuspendTime ,
#endif
};
// clang-format on

// ============================================================================
#endif  // STIME_IS_USED
