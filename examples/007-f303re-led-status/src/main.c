#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint( void ) {
    static int32_t loop = 220;
    float          data = -sin( ( double )loop / 180.0 * 3.1415926 );
    char*          ptr  = ( char* )&data;
    console.printf( "data = %f, ", data );
    for ( int i = 0; i < 4; i++ ) {
        console.printf( "%X ", *ptr++ );
    }
    console.printf( "\r\n" );
}

// ============================================================================
int main( void ) {
    utils.system.initClock( 72, 36, 72 );
    utils.system.initNvic( 4 );
    stime.config();
    stime.scheduler.config();
    console.config( 2000000, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_DOUBLE_BLINK );
    // tasks -----------
    stime.scheduler.attach( 500, 2, taskPrint, "taskPrint" );
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );
    return 0;
}
