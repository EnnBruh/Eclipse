#include "Layers/layers.h"

LayerID menu_layer_id;

void menu_layer_init(void) {
}

void menu_layer_term(void) {
}

void menu_layer_on_render(void) {
        render_rectangle_push(
                (f32vec2) { -0.5, 0.5 },
                (f32vec2) { 0.5, -0.5 },
                0xFFFFFFFF);
}

void menu_layer_on_update(f64 dt) {
}

void menu_layer_on_event(Event* event) {
}