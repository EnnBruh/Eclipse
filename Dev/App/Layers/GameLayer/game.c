#include "Layers/layers.h"
#include "Layers/GameLayer/player.h"

LayerID game_layer_id;

static f32vec2 offset;
static f32vec2 visible_tiles = {
        .x = 96,
        .y = 48 
};

static bool is_key_down[GLFW_KEY_LAST];
static Sprite background;

Player* local_player;
Player* remote_player;

static f32vec4 world_border;

void game_layer_init(void) {
        player_init_shadow();
        player_init_light();

        background = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 785 }, (i32vec2) { 385, 1000 });

        world_border = (f32vec4) {
                .x = -80,
                .y = 50,
                .z = 80,
                .w = -40
        };

        // local_player = &Shadow;
        // remote_player = &Light;
}

void game_layer_term(void) {
}

char fps_buff[16];
void game_layer_on_render(void) {
        render_proj_set((mat4) {
                2 / visible_tiles.x, 0, 0, -(2 * offset.x ) / visible_tiles.x,
                0, -(2 / visible_tiles.y), 0, (2 * offset.y) / visible_tiles.y,
                0, 0, -1, 0,
                0, 0, 0, 1
        });
        
        render_sprite_push(
                (f32vec2) { world_border.x, world_border.w },
                (f32vec2) { world_border.z, world_border.y },
                &background);
        
        
        player_render_shadow();
        player_render_light();


        render_proj_set((mat4) {
                1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
        });

        // render_rectangle_push(
        //         (f32vec2) { -1.0, -1.0 },
        //         (f32vec2) { 1.0, 1.0 },
        //         0xFF0000FF);

        render_text_push(
                (f32vec2) { -0.95, -0.9 },
                fps_buff, 0xFFFFFFFF, 0.05);

}

#define ENN_PLAYER_RUN_SPEED 10.0
#define ENN_PLAYER_GRAVITY 30.0
#define ENN_PLAYER_JUMP_FORCE 20.0

void game_layer_on_update(f64 dt) {
        sprintf(fps_buff, "%" PRIi32 " FPS", (i32)(1.0 / dt));
        // LOG("FrameTime = %lf FPS = %lf", dt, 1.0 / dt);

        if (is_key_down[GLFW_KEY_D]) local_player -> velocity.x = ENN_PLAYER_RUN_SPEED;
        else if (is_key_down[GLFW_KEY_A]) local_player -> velocity.x = -ENN_PLAYER_RUN_SPEED;
                else local_player -> velocity.x = 0.0;

        if (is_key_down[GLFW_KEY_SPACE])
                if ((local_player -> flags & (1 << ENN_PLAYER_ON_GROUND_BIT))) {
                        local_player -> velocity.y = -ENN_PLAYER_JUMP_FORCE;
                        local_player -> flags &= ~(1 << ENN_PLAYER_ON_GROUND_BIT);
                }
        
        
        if (!(local_player -> flags & (1 << ENN_PLAYER_ON_GROUND_BIT))) local_player -> velocity.y += ENN_PLAYER_GRAVITY * dt;
        if (fabs(local_player -> velocity.x) < 0.01) local_player -> velocity.x = 0.0;
        if (fabs(local_player -> velocity.y) < 0.01) local_player -> velocity.y = 0.0;

        f32vec2 new_pos = {
                .x = local_player -> pos.x + local_player -> velocity.x * dt,
                .y = local_player -> pos.y + local_player -> velocity.y * dt
        };

        if (new_pos.y > world_border.y - ENN_PLAYER_HEIGHT) {
                new_pos.y = world_border.y - ENN_PLAYER_HEIGHT, local_player -> velocity.y = 0.0;
                local_player -> flags |= (1 << ENN_PLAYER_ON_GROUND_BIT);
        }

        if (new_pos.x > world_border.z) {
                new_pos.x = world_border.z, local_player -> velocity.x = 0.0;
                // local_player -> flags |= (1 << ENN_PLAYER_ON_GROUND_BIT);
        }

        if (new_pos.x < world_border.x) {
                new_pos.x = world_border.x, local_player -> velocity.x = 0.0;
                // local_player -> flags |= (1 << ENN_PLAYER_ON_GROUND_BIT);
        }
        
        // LOG("velocity = %f %f", player_vel.x, player_vel.y);
        // LOG("pos = %f %f", player_pos.x, player_pos.y);
        local_player -> pos = new_pos;

        if (local_player -> velocity.y == 0.0) {
                if (local_player -> flags & (1 << ENN_PLAYER_ON_GROUND_BIT)) {
                        if (local_player -> velocity.x == 0.0) local_player -> state = ENN_IDLE;
                        else {
                                local_player -> state = ENN_RUNNING;
                                if (local_player -> velocity.x > 0.0)
                                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                                else if (local_player -> velocity.x < 0.0)
                                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
                        }
                } else {
                        local_player -> state = ENN_FALLING;
                        if (local_player -> velocity.x > 0.0)
                                local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                        else if (local_player -> velocity.x < 0.0)
                                local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
                }
        } else if (local_player -> velocity.y > 0.0) {
                local_player -> state = ENN_FALLING;
                if (local_player -> velocity.x > 0.0)
                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                else if (local_player -> velocity.x < 0.0)
                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
        } else {
                local_player -> state = ENN_JUMPING;
                if (local_player -> velocity.x > 0.0)
                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_EAST & (1 << ENN_PLAYER_FACING_BIT));
                else if (local_player -> velocity.x < 0.0)
                        local_player -> flags = (local_player -> flags & ~(1 << ENN_PLAYER_FACING_BIT)) | (ENN_PLAYER_FACING_WEST & (1 << ENN_PLAYER_FACING_BIT));
        }
        offset = local_player -> pos;

        static PlayerData data;
        if ((data.pos.x != local_player -> pos.x) || (data.pos.y != local_player -> pos.y) ||
            (data.flags != local_player -> flags) || (data.state != local_player -> state)) {
                data.pos = local_player->pos;
                data.flags = local_player->flags;
                data.state = local_player->state;
                network_packet_send(ENN_PKT_GAME_DATA, (const byte*)&data, (sizeof(PlayerData)));
        }
}

void game_layer_on_event(Event* event) {
        switch (event -> type) {
                case ENN_INPUT_KEY_EVENT:
                {
                        struct { i32 key, action; }* data = event -> data;
                        is_key_down[data -> key] = (data -> action == GLFW_PRESS || data -> action == GLFW_REPEAT);

                        if (data -> key == GLFW_KEY_ESCAPE && data -> action == GLFW_PRESS) {
                                layer_set_inactive(game_layer_id);
                                layer_set_active(menu_layer_id);
                        }
                        break;
                }
                case ENN_NETWORK_GAMEDATA_EVENT:
                {
                        PlayerData* data = event -> data;
                        remote_player -> pos = data -> pos;
                        remote_player -> state = data -> state;
                        remote_player -> flags = data -> flags;
                        break;
                }
                case ENN_GAME_SELECT_PLAYER_EVENT:
                {

                }
                default: break;
        }
}