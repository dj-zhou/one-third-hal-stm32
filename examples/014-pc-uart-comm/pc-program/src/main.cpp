#include "serial.h"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    ( void )argc;
    ( void )argv;
    Serial serial("/dev/ttyUSB0", 2000000, 'n');

    for (int i = 0; i < 10; i++) {
        const char* str = "hello world\n";
        serial.send(( char* )str, sizeof(str));
        sleep(1);
    }
    serial.quit();
    return 0;
}
