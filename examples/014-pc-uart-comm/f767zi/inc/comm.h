#pragma once

#include "config.h"
#include <stdlib.h>

void send_packet(UartApi_t*, void* msg, size_t msg_size);
