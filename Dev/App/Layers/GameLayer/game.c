#include "Layers/layers.h"
#include "Layers/GameLayer/player.h"

LayerID game_layer_id;

static f32vec2 offset;
static f32vec2 visible_tiles = {
        .x = 64,
        .y = 36
};

static f32vec2 window_size;
static bool is_key_down[GLFW_KEY_LAST];
static Sprite background;

void game_layer_init(void) {
        player_init_shadow();

        background = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 785 }, (i32vec2) { 385, 1000 });

        window_size.x = window_get_framebuff_width();
        window_size.y = window_get_framebuff_height();
        // LOG("%f %f %f %f", shadow_player_idle[0].texture_top_left.x, shadow_player_idle[0].texture_top_left.y, shadow_player_idle[0].texture_bott_right.x, shadow_player_idle[0].texture_bott_right.y);
}

void game_layer_term(void) {
}


void game_layer_on_render(void) {
        // render_proj_set((mat4) {
        //         1, 0, 0, 0,
        //         0, 1, 0, 0,
        //         0, 0, 1, 0,
        //         0, 0, 0, 1
        // });

        // render_rectangle_push(
        //         (f32vec2) { -1.0, 1.0 },
        //         (f32vec2) { 1.0, -1.0 },
        //         0x00FF00FF);

        // render_rectangle_push(
        //         (f32vec2) { -1.0, 1.0 },
        //         (f32vec2) { 1.0, -1.0 },
        //         0xFF000080);

        render_proj_set((mat4) {
                2 / visible_tiles.x, 0, 0, -(2 * offset.x ) / visible_tiles.x,
                0, -(2 / visible_tiles.y), 0, (2 * offset.y) / visible_tiles.y,
                0, 0, -1, 0,
                0, 0, 0, 1
        });
        
        render_sprite_push(
                (f32vec2) { -80, -70 },
                (f32vec2) { 80, 20 },
                &background);
        
        player_render_shadow();

}

#define ENN_PLAYER_RUN_SPEED 7.5
#define ENN_PLAYER_GRAVITY 20.0
#define ENN_PLAYER_JUMP_FORCE 15.0

void game_layer_on_update(f64 dt) {
        // LOG("FrameTime = %lf FPS = %lf", dt, 1.0 / dt);

        if (is_key_down[GLFW_KEY_D]) Shadow.velocity.x = ENN_PLAYER_RUN_SPEED;
        else if (is_key_down[GLFW_KEY_A]) Shadow.velocity.x = -ENN_PLAYER_RUN_SPEED;
                else Shadow.velocity.x = 0.0;

        if (is_key_down[GLFW_KEY_SPACE])
                if ((Shadow.flags & (1 << ENN_PLAYER_ON_GROUND_BIT))) {
                        Shadow.velocity.y = -ENN_PLAYER_JUMP_FORCE;
                        Shadow.flags &= ~(1 << ENN_PLAYER_ON_GROUND_BIT);
                }
        
        
        if (!(Shadow.flags & (1 << ENN_PLAYER_ON_GROUND_BIT))) Shadow.velocity.y += ENN_PLAYER_GRAVITY * dt;
        if (fabs(Shadow.velocity.x) < 0.01) Shadow.velocity.x = 0.0;
        if (fabs(Shadow.velocity.y) < 0.01) Shadow.velocity.y = 0.0;

        f32vec2 new_pos = {
                .x = Shadow.pos.x + Shadow.velocity.x * dt,
                .y = Shadow.pos.y + Shadow.velocity.y * dt
        };

        if (new_pos.y > 10.0) {
                new_pos.y = 10.0, Shadow.velocity.y = 0.0;
                Shadow.flags |= (1 << ENN_PLAYER_ON_GROUND_BIT);
        }
        
        // LOG("velocity = %f %f", player_vel.x, player_vel.y);
        // LOG("pos = %f %f", player_pos.x, player_pos.y);
        Shadow.pos = new_pos;

        if (Shadow.velocity.y == 0.0) {
                if (Shadow.flags & (1 << ENN_PLAYER_ON_GROUND_BIT)) {
                        if (Shadow.velocity.x == 0.0) Shadow.state = ENN_IDLE;
                        else {
                                Shadow.state = ENN_RUNNING;
                                if (Shadow.velocity.x > 0.0)
                                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                                else if (Shadow.velocity.x < 0.0)
                                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
                        }
                } else {
                        Shadow.state = ENN_FALLING;
                        if (Shadow.velocity.x > 0.0)
                                Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                        else if (Shadow.velocity.x < 0.0)
                                Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
                }
        } else if (Shadow.velocity.y > 0.0) {
                Shadow.state = ENN_FALLING;
                if (Shadow.velocity.x > 0.0)
                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                else if (Shadow.velocity.x < 0.0)
                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
        } else {
                Shadow.state = ENN_JUMPING;
                if (Shadow.velocity.x > 0.0)
                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                else if (Shadow.velocity.x < 0.0)
                        Shadow.flags = (Shadow.flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
        }
        offset = Shadow.pos;
}

void game_layer_on_event(Event* event) {
        switch (event -> type) {
                case ENN_INPUT_KEY_EVENT:
                {
                        struct { i32 key, action; }* data = event -> data;
                        is_key_down[data -> key] = (data -> action == GLFW_PRESS || data -> action == GLFW_REPEAT);
                        break;
                }
                case ENN_WINDOW_RESIZE_EVENT:
                {
                        i32vec2* data = event -> data;
                        window_size.x = data -> x;
                        window_size.y = data -> y;
                        break;
                }
                default: break;
        }
}