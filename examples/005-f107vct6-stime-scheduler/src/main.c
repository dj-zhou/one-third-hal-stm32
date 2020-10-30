
#include "config.h"
// =============================================================================
void task_printf( void ) {
    static int task_count = 0;
    Stime_t    time       = stime.getTime();
    console.printf( "%5d: hello world, time: %4d s, %4d ms\r\n", task_count++,
                    time.s, time.us / 1000 );
    // test for _1_TICK, _2_TICK and _3_TICK
    // console.printf( "%5d\r\n", task_count++ );
    utils.pin.toggle( GPIOD, 4 );
}

// ============================================================================
int main( void ) {
    utils.system.initClock();
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    stime.scheduler.config();
    console.config( 921600, 8, 'n', 1 );
    console.printf( "\r\n" );

    stime.scheduler.regist( 1000, 2, task_printf, "task_printf" );
    // also try _1_TICK, _2_TICK and _3_TICK
    // stime.registerTask( _2_TICK, 1, task_printf, "task_printf" );
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );

    return 0;
}
