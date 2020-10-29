
#include "config.h"

// ============================================================================
int main( void ) {
    utils.initSystemClock();
    utils.initNvic( 4 );
    utils.setPinMode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();

    while ( 1 ) {
        ;
    }
}
