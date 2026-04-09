#include "Layers/layers.h"

LayerID void_layer_id;

static i32vec4 window_viewport;

void void_layer_init(void) {
        i32vec2 window_dim = window_get_framebuff_dim();
        f64 original_aspect_ratio = (f64)16 / (f64)9;
        f64 new_aspect_ratio = (f64)window_dim.x / (f64)window_dim.y;

        if (new_aspect_ratio > original_aspect_ratio) {
                window_viewport.x = (f64)window_dim.x * 0.5 - original_aspect_ratio * (f64)window_dim.y * 0.5;
                window_viewport.y = 0;

                window_viewport.z = original_aspect_ratio * (f64)window_dim.y;
                window_viewport.w = window_dim.y;
        } else { 
                window_viewport.x = 0;
                window_viewport.y = (f64)window_dim.y * 0.5 - (f64)window_dim.x * 0.5 / original_aspect_ratio;

                window_viewport.z = window_dim.x;
                window_viewport.w = window_dim.x / original_aspect_ratio;
        }

        window_set_viewport(window_viewport.x, window_viewport.y, window_viewport.z, window_viewport.w);
} 

void void_layer_term(void) {
}

void void_layer_on_render(void) {
        render_buff_draw();
}

void void_layer_on_update(f64 dt) {
}

void void_layer_on_event(Event* event) {
        switch (event -> type) {
                case ENN_WINDOW_RESIZE_EVENT:
                {
                        i32vec2* window_dim = event -> data;

                        f64 original_aspect_ratio = (f64)16 / (f64)9;
                        f64 new_aspect_ratio = (f64)window_dim -> x / (f64)window_dim -> y;

                        if (new_aspect_ratio > original_aspect_ratio) {
                                window_viewport.x = (f64)window_dim -> x * 0.5 - original_aspect_ratio * (f64)window_dim -> y * 0.5;
                                window_viewport.y = 0;

                                window_viewport.z = original_aspect_ratio * (f64)window_dim -> y;
                                window_viewport.w = window_dim -> y;
                        } else { 
                                window_viewport.x = 0;
                                window_viewport.y = (f64)window_dim -> y * 0.5 - (f64)window_dim -> x * 0.5 / original_aspect_ratio;

                                window_viewport.z = window_dim -> x;
                                window_viewport.w = window_dim -> x / original_aspect_ratio;
                        }

                        window_set_viewport(window_viewport.x, window_viewport.y, window_viewport.z, window_viewport.w);
                        break;
                }
                default: break;
        }
}