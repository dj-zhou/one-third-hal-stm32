
#include "config.h"
#include <math.h>
// =============================================================================
void taskPrint( void ) {
    static int32_t loop = 0;
    double         data = -sin( ( double )loop / 180.0 * 3.1415926 );
    console.printf( "%5d: hello %s, data = %3.4f\r\n", loop++, FIRMWARE, data );
}

// ============================================================================
int main( void ) {
    utils.pin.mode( GPIOA, 5, GPIO_MODE_OUTPUT_PP );
    utils.system.initClock();
    utils.system.initNvic( 4 );
    stime.config();
    stime.scheduler.config();
    console.config( 2000000, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_DOUBLE_BLINK );
    // tasks -----------
    stime.scheduler.regist( 500, 2, taskPrint, "taskPrint" );
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );
    return 0;
}
