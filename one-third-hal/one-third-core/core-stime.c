#include "core-stime.h"

// ============================================================================
#if defined( STIME_IS_USED )
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

// ============================================================================
// constant values used accros the file -------------
// clang-format off
#if defined( _STIME_USE_SYSTICK )
    #if defined( _STIME_4K_TICK )
        #define    SYSTICK_1S_OVERFLOW     4000
        #define    SYSTICK_RELOAD_VALUE    2250
        #define    SYSTICK_MS_SCALE         250
    #elif defined( _STIME_2K_TICK )
        #define    SYSTICK_1S_OVERFLOW     2000
        #define    SYSTICK_RELOAD_VALUE    4500
        #define    SYSTICK_MS_SCALE         500
    #elif defined( _STIME_1K_TICK )
        #define    SYSTICK_1S_OVERFLOW     1000
        #define    SYSTICK_RELOAD_VALUE    9000
        #define    SYSTICK_MS_SCALE        1000
    #elif defined( _STIME_500_TICK )
        #define    SYSTICK_1S_OVERFLOW      500
        #define    SYSTICK_RELOAD_VALUE   18000
        #define    SYSTICK_MS_SCALE        2000
    #elif defined( _STIME_400_TICK )
        #define    SYSTICK_1S_OVERFLOW      400
        #define    SYSTICK_RELOAD_VALUE   22500
        #define    SYSTICK_MS_SCALE        2500
    #elif defined( _STIME_200_TICK )
        #define    SYSTICK_1S_OVERFLOW      200
        #define    SYSTICK_RELOAD_VALUE   45000
        #define    SYSTICK_MS_SCALE        5000
    #endif
#endif
// clang-format on

// ============================================================================
// SysTick is a 24-bit count down counter
// if FreeRTOS is used, this functions may be overwritten
static void InitSysTick( void ) {
    tick_   = 0;
    second_ = 0;
    HAL_SYSTICK_Config( SYSTICK_RELOAD_VALUE - 1U );
    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK_DIV8 );
    SET_BIT( SysTick->CTRL, SysTick_CTRL_TICKINT_Msk );
}

// ----------------------------------------------------------------------------
static Stime_t GetSysTickTime( void ) {
    // disable the SysTick, or use HAL_SuspendTick()
    CLEAR_BIT( SysTick->CTRL, SysTick_CTRL_TICKINT_Msk );
    uint32_t val = SysTick->VAL;
    Stime_t  st;
    st.s  = second_;
    st.us = tick_ * SYSTICK_MS_SCALE + ( SYSTICK_RELOAD_VALUE - val ) / 9;
    // enable the SysTick, or use HAL_ResumeTick()
    SET_BIT( SysTick->CTRL, SysTick_CTRL_TICKINT_Msk );
    return st;
}

// ----------------------------------------------------------------------------
// accurate now. but sometimes loose one tick?
static void DelayUs( uint32_t us ) {
    volatile Stime_t cur_time = GetSysTickTime();
    uint32_t         cur_us   = cur_time.s * 1000000 + ( uint32_t )cur_time.us;
    uint32_t         final_us = cur_us + ( uint32_t )us;
    do {
        cur_time.s  = second_;
        cur_time.us = tick_ * SYSTICK_MS_SCALE
                      + ( SYSTICK_RELOAD_VALUE - SysTick->VAL ) / 9;
        cur_us = cur_time.s * 1000000 + ( uint32_t )cur_time.us;
        // make sure it is not optimized
        for ( uint8_t i = 0; i < 2; i++ ) {
            __asm( "MOV R0,R0" );
        }
    } while ( cur_us <= final_us );
}

// ----------------------------------------------------------------------------
static void DelayMs( uint32_t ms ) {
    return DelayUs( ms * 1000 );
}

// ----------------------------------------------------------------------------
// when the interrupt is called, the value in SysTick->LOAD is reloaded to
// SysTick->VAL
void SysTick_Handler( void ) {
    tick_++;
    if ( tick_ >= SYSTICK_1S_OVERFLOW ) {
        tick_ = 0;
        second_++;
        // utils.togglePin( GPIOD, 4 );  // test only
    }
#ifdef _STIME_USE_SCHEDULER
    Task_Core_Remarks();  // todo
#endif
}
#endif  // STIME_IS_USED

// ============================================================================
#if defined( STIME_IS_USED )
// clang-format off
StimeApi_t stime = {
#if defined( _STIME_USE_SYSTICK )
    .config  = InitSysTick    ,
    .getTime = GetSysTickTime ,
    .delayUs = DelayUs        ,
    .delayMs = DelayMs        ,
#else
    #error StimeApi_t stime: not implemented.
#endif

#ifdef _STIME_USE_SCHEDULER
    .scheduler    = Scheduler_Config       ,
    .registerTask = Scheduler_Register_Task,
    .process      = Scheduler_Process      ,
    .showTasks    = Scheduler_Show_Tasks   ,
#endif
};
// clang-format on

// ============================================================================
#endif  // STIME_IS_USED
