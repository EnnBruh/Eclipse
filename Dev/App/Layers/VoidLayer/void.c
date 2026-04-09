#include "Layers/layers.h"

LayerID void_layer_id;

void void_layer_init(void) {
} 

void void_layer_term(void) {
}

void void_layer_on_render(void) {
        render_buff_draw();
}

void void_layer_on_update(f64 dt) {
}

void void_layer_on_event(Event* event) {
}