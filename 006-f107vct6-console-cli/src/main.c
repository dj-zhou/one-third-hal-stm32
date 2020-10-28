
#include "config.h"
// =============================================================================
void taskHeartBeat( void ) {
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

    stime.registerTask( 200, 2, taskHeartBeat, "taskHeartBeat" );
    // also try _1_TICK, _2_TICK and _3_TICK
    stime.showTasks();

    while ( 1 ) {
        console.cliProcess();
        stime.process();
    }
    return 0;
}
