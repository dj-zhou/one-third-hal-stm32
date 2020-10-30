
#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock();
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    console.config( 921600, 8, 'n', 1 );
    console.printf( "\r\n" );

    uint32_t loop_count = 0;
    Stime_t  time       = stime.getTime();
    Stime_t  time_old   = time;

    while ( 1 ) {
        time = stime.getTime();
        console.printf( "time.s = %5d, ", time.s );
        console.printf( "time.us = %6d, ", time.us );
        uint32_t diff =
            ( time.s - time_old.s ) * 1000000 + ( time.us - time_old.us );
        console.printf( "diff = %6ld us, diff2 = %3ld us\r\n", diff,
                        diff - 500000 );
        stime.delay.us( 500000 - 1075 );
        time_old = time;
        loop_count++;
    }
    return 0;
}
