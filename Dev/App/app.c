#include "app.h"
#include "Layers/layers.h"

void app_init(void) {
        DEBUG_TRACE();
        core_init();

        window_create(&(WindowSpecification) {
                .width = 800,
                .height = 600,
                .is_resizable = true
        });

        window_push_layer(&(Layer) {
                .active = true,
                .priority = 0,
                LAYER_ASSIGN(void)
        });

        window_push_layer(&(Layer) {
                .active = true,
                .priority = 1,
                LAYER_ASSIGN(network)
        });

        DEBUG_UNTRACE();
}

void app_run(void) {
        DEBUG_TRACE();
        core_run();
        DEBUG_UNTRACE();
}

void app_term(void) {
        DEBUG_TRACE();
        core_term();
        DEBUG_UNTRACE();
}