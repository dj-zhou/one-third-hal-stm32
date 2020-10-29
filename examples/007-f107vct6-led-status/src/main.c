
#include "config.h"
// =============================================================================
void taskPrint( void ) {
    static int32_t loop = 0;
    console.printf( "%5d: hello %s\r\n", loop++, FIRMWARE );
}

// ============================================================================
int main( void ) {
    utils.initSystemClock();
    utils.initNvic( 4 );
    utils.setPinMode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    stime.scheduler();
    console.config( 921600, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_DOUBLE_BLINK );

    // tasks -----------
    stime.registerTask( 1000, 2, taskPrint, "taskPrint" );
    stime.showTasks();

    while ( 1 ) {
        console.cliProcess();
        stime.process();
    }
    return 0;
}
