#ifndef __STIME_SCHEDULER_H
#define __STIME_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

// ============================================================================
// project interface --------------

// clang-format off
// can use some other timers for stime ---------------
#if defined( _STIME_USE_SYSTICK )
    #define STIME_IS_USED
#endif

// priority ---------------
#if defined( _STIME_USE_SYSTICK )
    #if !defined( _SYSTICK_PREEMPTION_PRIORITY )
        #define _SYSTICK_PREEMPTION_PRIORITY     1
    #endif
    #if !defined(_SYSTICK_SUB_PRIORITY )
        #define _SYSTICK_SUB_PRIORITY            0
    #endif
#endif

// tick frequency ---------------
// 200Hz, 400Hz, 500Hz, 1KHz, 2KHz, and 4KHz
#if !defined( _STIME_4K_TICK )  && \
    !defined( _STIME_2K_TICK )  && \
    !defined( _STIME_1K_TICK )  && \
    !defined( _STIME_500_TICK ) && \
    !defined( _STIME_400_TICK ) && \
    !defined( _STIME_200_TICK )
    #define _STIME_1K_TICK
#endif

// use task scheduler -----------------
#if defined( _STIME_USE_SCHEDULER )
    #if !defined( _STIME_TASK_MAX_NUM )
        #define _STIME_TASK_MAX_NUM     15
    #endif
    #if !defined( _STIME_TASK_NAME_LEN )
        #define _STIME_TASK_NAME_LEN    20
    #endif
    #if defined( _STIME_4K_TICK )
        #define _1_TICK   (UINT32_MAX - 1)
        #define _2_TICK   (UINT32_MAX - 2)
        #define _3_TICK   (UINT32_MAX - 3)
    #endif
    #if defined( _STIME_2K_TICK )
        #define _1_TICK   (UINT32_MAX - 4)
    #endif
#endif  // _STIME_USE_SCHEDULER
// clang-format on

// ============================================================================
// component data structure
// ----------------------------------------------------------------------------
#if defined( STIME_IS_USED )
typedef struct {
    uint32_t s;
    uint32_t us;
} Stime_t;
// ----------------------------------------------------------------------------
#if defined( _STIME_USE_SCHEDULER )
typedef void ( *TaskHandle )( void );

typedef struct TaskCell_s {
    uint8_t    run;    // 1: need to run, 0: do not need to run
    uint16_t   time;   // count down time, when 0, set run = 1
    uint16_t   ticks;  // total tick number for a loop
    char       name[_STIME_TASK_NAME_LEN];
    TaskHandle handle;
} TaskCell_t;

typedef struct TaskNode_s {
    TaskCell_t         _this;
    struct TaskNode_s* _next;
} TaskNode_t;
#endif  // _STIME_USE_SCHEDULER
#endif  // STIME_IS_USED

// ============================================================================
// component API
// clang-format off
#if defined( STIME_IS_USED )
typedef struct {
    void ( *us ) ( uint32_t );
    void ( *ms )  (uint32_t );
} Delay;

#if defined( _STIME_USE_SCHEDULER )
typedef struct  {
    void ( *config )( void )                                      ;
    void ( *regist )( uint32_t, uint32_t, TaskHandle, const char*);
    void ( *process )( void )                                     ;
    void ( *show )( void )                                        ;
    void ( *run )( void )                                         ;
} Scheduler;
#endif

typedef struct {
    void      ( *config )( void )     ;
    Stime_t   ( *getTime )( void )    ;
    Delay     delay                   ;
#if defined( _STIME_USE_SCHEDULER )
    Scheduler  scheduler              ;
#endif
} StimeApi_t;
extern StimeApi_t stime;
#endif // STIME_IS_USED
// clang-format on

// ============================================================================
#ifdef __cplusplus
}
#endif

#endif  // __STIME_SCHEDULER_H
