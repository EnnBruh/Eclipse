#include "Rendering/render.h"

#define ENN_PLAYER_FACING_EAST 1
#define ENN_PLAYER_FACING_WEST 0

#define ENN_PLAYER_FACING_BIT 0

#define ENN_PLAYER_ON_GROUND_BIT 1

#define ENN_PLAYER_TYPE_SHADOW 1
#define ENN_PLAYER_TYPE_LIGHT 0
#define ENN_PLAYER_TYPE_BIT 2

#define ENN_NUM_PLAYER_STATES 4

#define ENN_PLAYER_WIDTH 2
#define ENN_PLAYER_HEIGHT 3

#define ENN_MAX_ANIM_FRAMES 20

#define ENN_IDLE_ANIM_FPS 4
#define ENN_IDLE_ANIM_FRAMES 5

#define ENN_RUN_ANIM_FPS 8
#define ENN_RUN_ANIM_FRAMES 6

#define ENN_JUMP_ANIM_FPS 4
#define ENN_JUMP_ANIM_FRAMES 6

#define ENN_FALL_ANIM_FPS 4
#define ENN_FALL_ANIM_FRAMES 5

typedef enum ENN_PLAYER_STATE {
        ENN_IDLE,
        ENN_RUNNING,
        ENN_JUMPING,
        ENN_FALLING
} ENN_PLAYER_STATE;

typedef void (*RenderFn)(void);
typedef struct Player {
        f32vec2                 pos;
        f32vec2                 velocity;

        ENN_PLAYER_STATE        state;
        byte                    flags;

        RenderFn                render[ENN_NUM_PLAYER_STATES];
        Sprite                  sprites[ENN_NUM_PLAYER_STATES][ENN_MAX_ANIM_FRAMES];
} Player;

extern Player Light;
extern Player Shadow;

ENNDEF_PRIVATE void player_init_shadow(void);
ENNDEF_PRIVATE void player_init_light(void);

ENNDEF_PRIVATE void player_render_shadow(void);
ENNDEF_PRIVATE void player_render_light(void);

