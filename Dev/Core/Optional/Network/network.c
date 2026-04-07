#include "Optional/opt.h"

bool network_init() {
        DEBUG_TRACE();
#       if ENN_PLATFORM == ENN_WINDOWS
                WSADATA wsa;
                return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#       else
                return true;
#       endif
        DEBUG_UNTRACE();
}

void network_term() {
        DEBUG_TRACE();
#       if ENN_PLATFORM == ENN_WINDOWS
                WSACleanup();
#       endif
        DEBUG_UNTRACE();
}
