
#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock();
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );

    while ( 1 ) {
        // some delay ------------
        for ( int i = 0; i < 1000; i++ ) {
            for ( int j = 0; j < 1000; j++ ) {
                ;
            }
        }
        utils.pin.toggle( GPIOD, 4 );
    }
}
