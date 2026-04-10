#include "Layers/layers.h"

LayerID game_layer_id;

Sprite shadow_player_idle[5];
i32 idle_anim_index;

void game_layer_init(void) {
        shadow_player_idle[0] = (Sprite) { &global_render.sprite_sheet, (f32vec2) { 0, 16 },   (f32vec2) { 31, 63 } };
        shadow_player_idle[1] = (Sprite) { &global_render.sprite_sheet, (f32vec2) { 32, 16 },  (f32vec2) { 63, 63 } };
        shadow_player_idle[2] = (Sprite) { &global_render.sprite_sheet, (f32vec2) { 64, 16 },  (f32vec2) { 95, 63 } };
        shadow_player_idle[3] = (Sprite) { &global_render.sprite_sheet, (f32vec2) { 96, 16 },  (f32vec2) { 127, 63 } };
        shadow_player_idle[4] = (Sprite) { &global_render.sprite_sheet, (f32vec2) { 128, 16 }, (f32vec2) { 159, 63 } };
}

void game_layer_term(void) {
}

void game_layer_on_render(void) {
        static f64 last_anim_frame = 0.0;
        static f64 curr_anim_frame = 0.0;
        curr_anim_frame = glfwGetTime();
        
        if (curr_anim_frame - last_anim_frame >= 0.25 ) {
                idle_anim_index = (idle_anim_index + 1) % 5;
                last_anim_frame = curr_anim_frame;
        }

        render_rectangle_push(
                (f32vec2) { -1.0, 1.0 },
                (f32vec2) { 1.0, -1.0 },
                0x00FF00FF);

        render_rectangle_push(
                (f32vec2) { -1.0, 1.0 },
                (f32vec2) { 1.0, -1.0 },
                0xFF000080);

        render_sprite_push_color(
                ((f32vec2) { -0.1, 0.2 }),
                ((f32vec2) { 0.1, -0.2}),
                &shadow_player_idle[idle_anim_index], 0xFF0000FF);
}

void game_layer_on_update(f64 dt) {
        // LOG("FrameTime = %lf FPS = %lf", dt, 1.0 / dt);
}

void game_layer_on_event(Event* event) {
}