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

        render_init();

        void_layer_id = window_push_layer(&(Layer) {
                .active = true,
                .priority = 0,
                LAYER_ASSIGN(void)
        });

        debug_layer_id = window_push_layer(&(Layer) {
                .active = true,
                .priority = 100,
                LAYER_ASSIGN(debug)
        });

        network_layer_id = window_push_layer(&(Layer) {
                .active = true,
                .priority = 1,
                LAYER_ASSIGN(network)
        });

        menu_layer_id = window_push_layer(&(Layer) {
                .active = false,
                .priority = 2,
                LAYER_ASSIGN(menu)
        });

        game_layer_id = window_push_layer(&(Layer) {
                .active = true,
                .priority = 2,
                LAYER_ASSIGN(game)
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
        render_term();
        core_term();
        DEBUG_UNTRACE();
}