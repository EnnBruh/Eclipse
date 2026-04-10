#include "Layers/layers.h"

LayerID menu_layer_id;

void menu_layer_init(void) {
}

void menu_layer_term(void) {
}

void menu_layer_on_render(void) {
        LOG("IF U SEE THIS ITS BAD");
        render_rectangle_push(
                (f32vec2) { -1.0, 1.0 },
                (f32vec2) { 1.0, -1.0 },
                0xFF0000FF);
}

void menu_layer_on_update(f64 dt) {
}

void menu_layer_on_event(Event* event) {
}