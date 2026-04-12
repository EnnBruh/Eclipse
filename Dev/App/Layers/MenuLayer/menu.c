#include "Layers/layers.h"


LayerID menu_layer_id;

static Sprite background;
static vector(Button) buttons;

#define BUTTON_DEFAULT_COLOR 0xFFFFFFFF
#define BUTTON_HOVER_COLOR 0x4d4c59FF

void menu_layer_init(void) {
        background = render_sprite_create(&global_render.sprite_sheet,
                (i32vec2) { 0, 567 },
                (i32vec2) { 383, 783 });
        
        vector_reserve(buttons, 3);
        Button butt = {
                .pos = (f32vec2) { -0.9, 0.0 },
                .dim = (f32vec2) { 0.15, 0.075 },
                .name = "PLAY",
                .color = BUTTON_DEFAULT_COLOR
        };
        vector_push_back(buttons, butt);

        butt.pos.y = 0.1;
        butt.name = "SETTINGS";
        butt.dim.x = 0.3;

        vector_push_back(buttons, butt);

        butt.pos.y = 0.2;
        butt.name = "QUIT";
        butt.dim.x = 0.15;

        vector_push_back(buttons, butt);
}

void menu_layer_term(void) {
        vector_destroy(buttons);
}

void menu_layer_on_render(void) {
        render_proj_set((mat4) {
                1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
        });


        render_sprite_push(
                (f32vec2) { -1.0, -1.0 },
                (f32vec2) { 1.0, 1.0 },
                &background);

        render_text_push(
                (f32vec2) { -0.905, -0.195 },
                "ECLIPSE", 0xFFFFFFFF, 0.1);

        for (i32 i = buttons.start; i < buttons.end; ++i)
                ui_button_render(&buttons.data[i]);
}

void menu_layer_on_update(f64 dt) {
}

ENNDEF_PUBLIC f32vec2 screen_to_ndc(f32vec2 screen) {
        return (f32vec2) {
                .x = (screen.x - window_viewport.x) / ((f32)(window_viewport.z) * 0.5) - 1.0,
                .y = (screen.y - window_viewport.y) / ((f32)(window_viewport.w) * 0.5) - 1.0
        };
}

void menu_layer_on_event(Event* event) {
        switch (event -> type) {
                case ENN_INPUT_MOUSE_MOVE_EVENT:
                {
                        struct { f64vec2 screen; }* data = event -> data; 
                        f32vec2 ndc = screen_to_ndc((f32vec2) { data -> screen.x, data -> screen.y });

                        // LOG("screen = %lf %lf", data -> screen.x, data -> screen.y);
                        // LOG("ndc = %f %f", ndc.x, ndc.y);

                        for (i32 i = buttons.start; i < buttons.end; ++i)
                                if (is_button_under(buttons.data[i], ndc)) buttons.data[i].color = BUTTON_HOVER_COLOR;
                                else buttons.data[i].color = BUTTON_DEFAULT_COLOR;
                        break;
                }
                case ENN_INPUT_MOUSE_BUTTON_EVENT:
                {
                        struct { i32 button, action; }* data = event -> data;
                        if (data -> button == GLFW_MOUSE_BUTTON_LEFT && data -> action == GLFW_PRESS) {
                                for (i32 i = buttons.start; i < buttons.end; ++i)
                                        if (buttons.data[i].color == BUTTON_HOVER_COLOR) {
                                                if (strcmp(buttons.data[i].name, "PLAY") == 0) {
                                                        layer_set_inactive(menu_layer_id);
                                                        if (net_state.status == ENN_NETWORK_DISCONNECTED) layer_set_active(network_layer_id);
                                                        else layer_set_active(game_layer_id);
                                                } else if (strcmp(buttons.data[i].name, "SETTINGS") == 0) {
                                                } else if (strcmp(buttons.data[i].name, "QUIT") == 0) {
                                                        core_stop();
                                                }
                                                break;
                                        }
                        }
                        break;
                }
                default: break;
        }
}