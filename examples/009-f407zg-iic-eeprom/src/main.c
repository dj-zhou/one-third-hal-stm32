#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint( void ) {
    static int32_t loop = 220;
    float          data = -sin( ( double )loop / 180.0 * 3.1415926 );
    char*          ptr  = ( char* )&data;
    console.printf( "data = %f, ", data );
    for ( int i = 0; i < 4; i++ ) {
        console.printf( "%X ", *ptr++ );
    }
    console.printf( "\r\n" );
}

// ============================================================================
int main( void ) {
    utils.system.initClock( 168, 42, 84 );
    utils.system.initNvic( 4 );
    stime.config();
    stime.scheduler.config();
    console.config( 2000000, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_PWM_MODE );
    iic1.config( 400000 );
    eeprom.config();
    // tasks -----------
    stime.scheduler.attach( 500, 2, taskPrint, "taskPrint" );
    stime.scheduler.show();

    uint8_t data_to_write[1024];
    uint8_t data_to_read[1024];
    uint8_t data_to_compare[1024];
    for ( uint16_t i = 0; i < 1024; i++ ) {
        data_to_write[i] = i + 10;
    }
    uint32_t total_bytes_to_write = 200;

    // test 1: write to eeprom byte by byte --------------
    console.printf( "--------------------------------\r\n%s: write to eeprom "
                    "byte by byte of %d bytes, need %d "
                    "seconds to wait:\r\n",
                    __TIME__, total_bytes_to_write,
                    total_bytes_to_write * 10 / 1000 );
    for ( uint16_t i = 0; i < total_bytes_to_write; i++ ) {
        eeprom.writeByte( i, data_to_write[i] );
        stime.delay.ms( 10 );
    }
    for ( uint16_t i = 0; i < total_bytes_to_write; i++ ) {
        data_to_read[i] = eeprom.readByte( i );
        stime.delay.ms( 1 );
    }
    uint32_t sum = 0;
    for ( uint16_t i = 0; i < total_bytes_to_write; i++ ) {
        data_to_compare[i] = data_to_read[i] - data_to_write[i];
        sum += data_to_compare[i];
    }
    console.printf( "compare sum = %d\r\n", sum );
    if ( sum == 0 ) {
        console.printf( "writing and reading are successful!\r\n" );
    }

    // test 2: write to eeprom at once --------------
    console.printf( "--------------------------------\r\n%s: write to eeprom "
                    "%d bytes, no need to wait:\r\n",
                    __TIME__, total_bytes_to_write );
    for ( uint16_t i = 0; i < 1024; i++ ) {
        data_to_write[i] = i + 15;
    }
    eeprom.writeNbytes( 0, data_to_write, total_bytes_to_write );
    eeprom.readNbytes( 0, data_to_read, total_bytes_to_write );
    sum = 0;
    for ( uint16_t i = 0; i < total_bytes_to_write; i++ ) {
        data_to_compare[i] = data_to_read[i] - data_to_write[i];
        sum += data_to_compare[i];
    }
    console.printf( "compare sum = %d\r\n", sum );
    if ( sum == 0 ) {
        console.printf( "writing and reading are successful!\r\n" );
    }
    while ( 1 )
        ;
    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );
    return 0;
}
