#include "config.h"

// ============================================================================
int main( void ) {
    utils.system.initClock();
    utils.system.initNvic( 4 );
    utils.pin.mode( GPIOD, 4, GPIO_MODE_OUTPUT_PP );
    stime.config();
    console.config( 921600, 8, 'n', 1 );
    static int loop_count = 0;

    while ( 1 ) {
        // just some delay
        for ( int i = 0; i < 3000; i++ ) {
            for ( int j = 0; j < 3000; j++ ) {
                ;
            }
        }
        console.printf(
            "\r\n\r\n---------------------------------------------\r\n" );
        utils.pin.toggle( GPIOD, 4 );

        console.printf( YLW "%s\r\n " NOC, FIRMWARE );
        console.printf( "%5d = %b\r\n", loop_count++, loop_count );
        float pi = 3.1415926;
        console.printf( GRN "pi = %f\r\n" NOC, pi );
        const char str[] = "this is a one-third demo program";
        console.printf( " %s\r\n", str );
        int data_int = 3752;
        console.printf( " data_int = %d, %o, %0X\r\n", data_int, data_int,
                        data_int );
        console.printf( " data_int = %0x, %ld, %lu\r\n", data_int, data_int,
                        data_int );
    }
}
