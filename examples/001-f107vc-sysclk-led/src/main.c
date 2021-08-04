
#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock( 72, 36, 72 );
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
#ifdef __DEBUG
#define j_max 500
#else
#define j_max 2000
#endif
    while ( 1 ) {
        // some delay ------------
        for ( int i = 0; i < 1000; i++ ) {
            for ( int j = 0; j < j_max; j++ ) {
                ;
            }
        }
        utils.pin.toggle( GPIOD, 4 );
    }
}
