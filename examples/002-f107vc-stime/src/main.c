
#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock( 72, 36, 72 );
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();

    while ( 1 ) {
        ;
    }
}
