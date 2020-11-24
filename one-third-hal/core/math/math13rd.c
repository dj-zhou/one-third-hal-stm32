#include "math13rd.h"

// ============================================================================
int to_lower( int c ) {
    if ( c >= 'A' && c <= 'Z' ) {
        return c + 'a' - 'A';
    }
    else {
        return c;
    }
}

// ============================================================================
int to_upper( int c ) {
    if ( c >= 'a' && c <= 'z' ) {
        return c + 'A' - 'a';
    }
    else {
        return c;
    }
}

// ============================================================================
int htoi( char s[] ) {
    int i;
    int n = 0;
    if ( s[0] == '0' && ( s[1] == 'x' || s[1] == 'X' ) ) {
        i = 2;
    }
    else {
        i = 0;
    }
    for ( ; ( s[i] >= '0' && s[i] <= '9' ) || ( s[i] >= 'a' && s[i] <= 'z' )
            || ( s[i] >= 'A' && s[i] <= 'Z' );
          ++i ) {
        if ( to_lower( s[i] ) > '9' ) {
            n = 16 * n + ( 10 + to_lower( s[i] ) - 'a' );
        }
        else {
            n = 16 * n + ( to_lower( s[i] ) - '0' );
        }
    }
    return n;
}
