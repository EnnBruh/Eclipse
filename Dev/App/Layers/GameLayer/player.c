#include "Layers/GameLayer/player.h"

Player Shadow;
Player Light;

ENNDEF_PUBLIC void player_render_shadow_idle(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_IDLE_ANIM_FPS) {
                anim_index = (anim_index + 1) % ENN_IDLE_ANIM_FRAMES;
                last_frame = curr_frame;
        }

        if (!(Shadow.flags & (1 << ENN_PLAYER_FACING_BIT))) {
                render_sprite_flip_vertical(&Shadow.sprites[ENN_IDLE][anim_index]);

                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_IDLE][anim_index]);

                render_sprite_flip_vertical(&Shadow.sprites[ENN_IDLE][anim_index]);
        } else {
                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_IDLE][anim_index]);
        }
}

ENNDEF_PUBLIC void player_render_shadow_run(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_RUN_ANIM_FPS) {
                anim_index = (anim_index + 1) % ENN_RUN_ANIM_FRAMES;
                last_frame = curr_frame;
        }


        if (!(Shadow.flags & (1 << ENN_PLAYER_FACING_BIT))) {
                render_sprite_flip_vertical(&Shadow.sprites[ENN_RUNNING][anim_index]);

                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_RUNNING][anim_index]);

                render_sprite_flip_vertical(&Shadow.sprites[ENN_RUNNING][anim_index]);
        } else {
                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_RUNNING][anim_index]);
        }
}

ENNDEF_PUBLIC void player_render_shadow_jump(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_JUMP_ANIM_FPS) {
                if (anim_index + 1 == ENN_JUMP_ANIM_FRAMES);
                else anim_index += 1;
                last_frame = curr_frame;
        }

        if (!(Shadow.flags & (1 << ENN_PLAYER_FACING_BIT))) {
                render_sprite_flip_vertical(&Shadow.sprites[ENN_JUMPING][anim_index]);

                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_JUMPING][anim_index]);

                render_sprite_flip_vertical(&Shadow.sprites[ENN_JUMPING][anim_index]);
        } else {
                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_JUMPING][anim_index]);
        }
}

ENNDEF_PUBLIC void player_render_shadow_fall(void) {
        static f32 last_frame = 0.0;
        static f32 curr_frame = 0.0;
        static i32 anim_index = 0;

        curr_frame = glfwGetTime();
        if (curr_frame - last_frame >= (f32)1.0 / (f32)ENN_FALL_ANIM_FPS) {
                if (anim_index + 1 == ENN_FALL_ANIM_FRAMES);
                else anim_index += 1;
                last_frame = curr_frame;
        }

        if (!(Shadow.flags & (1 << ENN_PLAYER_FACING_BIT))) {
                render_sprite_flip_vertical(&Shadow.sprites[ENN_FALLING][anim_index]);

                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_FALLING][anim_index]);

                render_sprite_flip_vertical(&Shadow.sprites[ENN_FALLING][anim_index]);
        } else {
                render_sprite_push(Shadow.pos, 
                        (f32vec2) { Shadow.pos.x + ENN_PLAYER_WIDTH, Shadow.pos.y + ENN_PLAYER_HEIGHT },
                        &Shadow.sprites[ENN_FALLING][anim_index]);
        }
}

void player_init_shadow(void) {
        Shadow.state = ENN_IDLE;

        Shadow.render[ENN_IDLE] = player_render_shadow_idle;
        Shadow.render[ENN_RUNNING] = player_render_shadow_run;
        Shadow.render[ENN_JUMPING] = player_render_shadow_jump;
        Shadow.render[ENN_FALLING] = player_render_shadow_fall;

        Shadow.sprites[ENN_IDLE][0] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 16 },   (i32vec2) { 32, 64 });
        Shadow.sprites[ENN_IDLE][1] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 32, 16 },  (i32vec2) { 64, 64 });
        Shadow.sprites[ENN_IDLE][2] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 64, 16 },  (i32vec2) { 96, 64 });
        Shadow.sprites[ENN_IDLE][3] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 96, 16 },  (i32vec2) { 128, 64 });
        Shadow.sprites[ENN_IDLE][4] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128, 16 }, (i32vec2) { 160, 64 });

        Shadow.sprites[ENN_RUNNING][0] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 16 + 48 *2 },   (i32vec2) { 32, 64 + 48* 2});
        Shadow.sprites[ENN_RUNNING][1] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 32, 16 + 48* 2},  (i32vec2) { 64, 64 + 48* 2});
        Shadow.sprites[ENN_RUNNING][2] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 64, 16 + 48 * 2},  (i32vec2) { 96, 64 + 48* 2});
        Shadow.sprites[ENN_RUNNING][3] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 96, 16 + 48 * 2},  (i32vec2) { 128, 64 + 48*2});
        Shadow.sprites[ENN_RUNNING][4] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128, 16 + 48*2}, (i32vec2) { 160, 64 + 48*2});
        Shadow.sprites[ENN_RUNNING][5] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128 + 32, 16 + 48* 2}, (i32vec2) { 160 + 32, 64 + 48*2});

        Shadow.sprites[ENN_JUMPING][0] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 16 + 48 * 4 },   (i32vec2) { 32, 64 + 48* 4});
        Shadow.sprites[ENN_JUMPING][1] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 32, 16 + 48 * 4},  (i32vec2) { 64, 64 + 48* 4});
        Shadow.sprites[ENN_JUMPING][2] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 64, 16 + 48 * 4},  (i32vec2) { 96, 64 + 48* 4});
        Shadow.sprites[ENN_JUMPING][3] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 96, 16 + 48 * 4},  (i32vec2) { 128, 64 + 48 * 4});
        Shadow.sprites[ENN_JUMPING][4] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128, 16 + 48 * 4}, (i32vec2) { 160, 64 + 48 * 4});
        Shadow.sprites[ENN_JUMPING][5] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128 + 32, 16 + 48 * 4}, (i32vec2) { 160 + 32, 64 + 48 * 4});

        Shadow.sprites[ENN_FALLING][0] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 0, 16 + 48 * 5 },   (i32vec2) { 32, 64 + 48* 5});
        Shadow.sprites[ENN_FALLING][1] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 32, 16 + 48 * 5},  (i32vec2) { 64, 64 + 48* 5});
        Shadow.sprites[ENN_FALLING][2] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 64, 16 + 48 * 5},  (i32vec2) { 96, 64 + 48* 5});
        Shadow.sprites[ENN_FALLING][3] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 96, 16 + 48 * 5},  (i32vec2) { 128, 64 + 48 * 5});
        Shadow.sprites[ENN_FALLING][4] = render_sprite_create(&global_render.sprite_sheet, (i32vec2) { 128, 16 + 48 * 5}, (i32vec2) { 160, 64 + 48 * 5});
}

void player_render_shadow(void) {
        Shadow.render[Shadow.state]();
}
