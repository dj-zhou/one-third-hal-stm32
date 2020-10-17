
#include "config.h"

// ============================================================================
int main( void ) {
    utils.initSystemClock();
    utils.initNvic( 4 );
    utils.setPinMode( GPIOC, 6, GPIO_MODE_OUTPUT_PP );

    while ( 1 ) {
        // some delay ------------
        for ( int i = 0; i < 1000; i++ ) {
            for ( int j = 0; j < 1000; j++ ) {
                ;
            }
        }
        utils.togglePin( GPIOC, 6 );
    }
}
