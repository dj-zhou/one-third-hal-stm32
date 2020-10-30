
#include "config.h"
// =============================================================================
void taskPrint( void ) {
    static int32_t loop = 0;
    console.printf( "%5d: hello %s\r\n", loop++, FIRMWARE );
}

// ============================================================================
int main( void ) {
    utils.system.initClock();
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    stime.scheduler.config();
    console.config( 921600, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_DOUBLE_BLINK );

    // tasks -----------
    stime.scheduler.regist( 1000, 2, taskPrint, "taskPrint" );
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );
    return 0;
}
