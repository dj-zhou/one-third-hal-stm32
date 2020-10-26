
#include "config.h"
// =============================================================================
void task_printf( void ) {
    static int task_count = 0;
    Stime_t    time       = stime.getTime();
    console.printf( "%5d: hello world, time: %4d s, %4d ms\r\n", task_count++,
                    time.s, time.us / 1000 );
    // test for _1_TICK, _2_TICK and _3_TICK
    // console.printf( "%5d\r\n", task_count++ );
    utils.togglePin( GPIOD, 4 );
}

// ============================================================================
int main( void ) {
    utils.initSystemClock();
    utils.initNvic( 4 );
    utils.setPinMode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    stime.scheduler();
    console.config( 921600, 8, 'n', 1 );
    console.printf( "\r\n" );

    stime.registerTask( 1000, 2, task_printf, "task_printf" );
    // also try _1_TICK, _2_TICK and _3_TICK
    // stime.registerTask( _2_TICK, 1, task_printf, "task_printf" );
    stime.showTasks();

    while ( 1 ) {
        stime.process();
    }
    return 0;
}
