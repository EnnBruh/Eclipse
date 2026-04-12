#include "Layers/layers.h"

LayerID select_layer_id;

ENNDEF_PUBLIC void select_shadow_render(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_IDLE_ANIM_FPS) {
                anim_index = (anim_index + 1) % ENN_IDLE_ANIM_FRAMES;
                last_frame = curr_frame;
        }

        render_sprite_flip_horizontal(&Shadow.sprites[ENN_IDLE][anim_index]);

        render_sprite_push(
                (f32vec2) { -0.7, -0.7 },
                (f32vec2) { -0.2,  0.7 },
                &Shadow.sprites[ENN_IDLE][anim_index]);

        render_sprite_flip_horizontal(&Shadow.sprites[ENN_IDLE][anim_index]);

        render_text_push(
                (f32vec2) { -0.6, 0.75 },
                "SHADOW", 0xFFFFFFFF, 0.1);
}

ENNDEF_PUBLIC void select_light_render(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_IDLE_ANIM_FPS) {
                anim_index = (anim_index + 1) % ENN_IDLE_ANIM_FRAMES;
                last_frame = curr_frame;
        }

        render_sprite_push(
                (f32vec2) { 0.2, -0.7 },
                (f32vec2) { 0.7,  0.7 },
                &Light.sprites[ENN_IDLE][anim_index]);

        render_text_push(
                (f32vec2) { 0.325, 0.75 },
                "LIGHT", 0xFFFFFFFF, 0.1);
}

static Button light_option;
static Button shadow_option;
void select_layer_init(void) {
        light_option.pos = (f32vec2) { 0.2, -0.7 };
        light_option.dim = (f32vec2) { 0.5,  1.45 };

        shadow_option.pos = (f32vec2) { -0.7, -0.7 };
        shadow_option.dim = (f32vec2) { 0.5,  1.45 };
}

void select_layer_term(void) {
}

static char error[256];
void select_layer_on_render(void) {
        render_text_push(
                (f32vec2) { -0.525, -0.85 },
                "CHOOSE YOUR CHARACTER", 0xFFFFFFFF, 0.1);

        render_text_push(
                (f32vec2) { -0.525, -0.75 },
                error, 0xa1274eFF, 0.05);

        select_shadow_render();
        if (shadow_option.hover) render_rectangle_push(shadow_option.pos, (f32vec2) { shadow_option.pos.x + shadow_option.dim.x, shadow_option.pos.y + shadow_option.dim.y }, 0xFFFFFF50);
        select_light_render();
        if (light_option.hover) render_rectangle_push(light_option.pos, (f32vec2) { light_option.pos.x + light_option.dim.x, light_option.pos.y + light_option.dim.y }, 0xFFFFFF50);
}

void select_layer_on_update(f64 dt) {
}

void select_layer_on_event(Event* event) {
        switch (event -> type) {
                case ENN_INPUT_MOUSE_MOVE_EVENT:
                {
                        struct { f64vec2 pos; }* data = event -> data;
                        f32vec2 ndc = screen_to_ndc((f32vec2) { data -> pos.x, data -> pos.y });
                        if (is_button_under(light_option, ndc)) light_option.hover = true;
                        else light_option.hover = false;
                        if (is_button_under(shadow_option, ndc)) shadow_option.hover = true;
                        else shadow_option.hover = false;
                        break;
                }
                case ENN_INPUT_MOUSE_BUTTON_EVENT:
                {
                        struct { i32 button, action; }* data = event -> data;
                        if (data -> button == GLFW_MOUSE_BUTTON_LEFT && data -> action == GLFW_PRESS) {
                                if (light_option.hover) {
                                        if (remote_player == NULL || remote_player != &Light) {
                                                local_player = &Light;
                                                error[0] = '\0';
                                                SelectData data;
                                                data.choice = ENN_CHOICE_LIGHT;
                                                network_packet_send(ENN_PKT_SELECT_DATA, (const byte*)&data, (sizeof (SelectData)));
                                                layer_set_inactive(select_layer_id);
                                                layer_set_active(game_layer_id);
                                        } else if (remote_player == &Light) {
                                                sprintf(error, "ERROR: YOUR FRIEND HAS ALREADY PICKED LIGHT");
                                        }
                                }
                                else if (shadow_option.hover) {
                                        if (remote_player == NULL || remote_player != &Shadow) {
                                                local_player = &Shadow;
                                                error[0] = '\0';
                                                SelectData data;
                                                data.choice = ENN_CHOICE_SHADOW;
                                                network_packet_send(ENN_PKT_SELECT_DATA, (const byte*)&data, (sizeof (SelectData)));
                                                layer_set_inactive(select_layer_id);
                                                layer_set_active(game_layer_id);
                                        } else if (remote_player == &Shadow) {
                                                sprintf(error, "ERROR: YOUR FRIEND HAS ALREADY PICKED SHADOW");
                                        }
                                }
                        }
                        break;
                }
                case ENN_NETWORK_GAMEDATA_EVENT:
                {
                        SelectData* data = event -> data;
                        if (data -> choice == ENN_CHOICE_LIGHT) remote_player = &Light;
                        else if (data -> choice == ENN_CHOICE_SHADOW) remote_player = &Shadow;
                        break;
                }
                default: break;
        }
}