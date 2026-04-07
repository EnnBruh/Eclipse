#include "core.h"

signed main(void) {
        DEBUG_TRACE();
        core_init();

        window_create(&(WindowSpecification) {
                .width = 800,
                .height = 600,
                .is_resizable = true
        });

        core_run();

        core_term();
        DEBUG_UNTRACE();
        return 0;
}
