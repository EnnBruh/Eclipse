#include "app.h"

signed main(void) {
        DEBUG_TRACE();
        app_init();
        app_run();
        app_term();
        DEBUG_UNTRACE();
        return 0;
}
