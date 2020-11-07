
#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock( 168, 42, 84 );
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOE, 11, GPIO_MODE_OUTPUT_PP );

    while ( 1 ) {
        // some delay ------------
        for ( int i = 0; i < 1000; i++ ) {
            for ( int j = 0; j < 1000; j++ ) {
                ;
            }
        }
        utils.pin.toggle( GPIOE, 11 );
    }
}
