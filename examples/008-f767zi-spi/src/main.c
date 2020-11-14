#include "config.h"
#include <math.h>

// =============================================================================
void taskPrint( void ) {
    static int32_t  loop       = 220;
    float           data       = -sin( ( double )loop / 180.0 * 3.1415926 );
    char*           ptr        = ( char* )&data;
    static uint32_t loop_count = 0;
    console.printf( "-------------------\r\n" );
    console.printf( "%5d,data = %f, ", loop_count++, data );
    for ( int i = 0; i < 4; i++ ) {
        console.printf( "%X ", *ptr++ );
    }
    console.printf( "\r\n" );
    uint32_t addr1 = ( uint32_t )&spi1;
    uint32_t addr2 = ( uint32_t )&spi1.transceive;
    console.printf( "address of spi1 = %08d\r\n", addr1 );
    console.printf( "address of spi1.transceive = %08d\r\n", addr2 );
    console.printf( "address diff = %d\r\n", addr2 - addr1 );
    Spi_t* spi = container_of( &( spi1.transceive ), Spi_t, transceive );
    console.printf( "address of spi = %08d\r\n", ( uint32_t )spi );
}

// ============================================================================
void taskSpiTransceive( void ) {
    static uint32_t loop_count = 0;
    console.printf( "-------------------\r\ntaskSpiTransceive\r\n" );
    uint8_t tbuf[8];
    uint8_t rbuf[8];
    for ( int i = 0; i < 8; i++ ) {
        tbuf[i] = i * 2 + loop_count;
        console.printf( "%3d ", tbuf[i] );
    }
    console.printf( "\r\n" );
    spi1.transceive( tbuf, rbuf, 8 );
    loop_count++;
}

// ============================================================================
int main( void ) {
    utils.system.initClock( 216, 54, 108 );
    utils.system.initNvic( 4 );
    stime.config();
    stime.scheduler.config();
    console.config( 2000000, 8, 'n', 1 );
    console.printf( "\r\n\r\n" );
    led.config( LED_DOUBLE_BLINK );
    spi1.config( 32, SPI_MASTER, SPI_SOFT_NSS );
    spi1.setNss( GPIOC, 2 );

    // tasks -----------
    stime.scheduler.attach( 500, 2, taskPrint, "taskPrint" );
    stime.scheduler.attach( 1000, 100, taskSpiTransceive, "taskSpiTransceive" );
    stime.scheduler.show();

    // system start to run -----------
    stime.scheduler.run();

    console.printf( "main ends.\r\n" );
    return 0;
}
