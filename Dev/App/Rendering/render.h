#include "core.h"

typedef struct Vertex {
        f32vec2         pos;
        u32             color;
        f32vec2         texture_pos;
} Vertex;


#define ENN_RENDER_VERTEX_BUFF_SIZE 16384

typedef GLint UniformLocation;

extern struct Renderer {
        Vertex          buff[2][ENN_RENDER_VERTEX_BUFF_SIZE];
        i32             buff_size[2];

        VAOID           vao;
        VBOID           vbo;
        ShaderID        shader;

        UniformLocation proj_matrix_location;

        TextureID       sprite_sheet;
        TextureID       font_atlas;
} global_render;

typedef struct Sprite {
        f32vec2 texture_pos;
} Sprite;

ENNDEF_PRIVATE void render_init(void);
ENNDEF_PRIVATE void render_term(void);
ENNDEF_PRIVATE void render_buff_draw(void);

// ENNDEF_PRIVATE void render_proj_set(const f32* proj_matrix);

ENNDEF_PRIVATE void render_rectangle_push(f32vec2 top_left, f32vec2 bott_right, u32 color);
ENNDEF_PRIVATE void render_line_push(f32vec2 pos1, f32vec2 pos2, f32 width, u32 color);
ENNDEF_PRIVATE void render_sprite_push_4(f32vec2 top_left, f32vec2 bott_right, Sprite sprite, u32 color);
ENNDEF_PRIVATE void render_sprite_push_3(f32vec2 top_left, f32vec2 bott_right, Sprite sprite);
#define render_sprite_push(...) EXPAND(JOIN(render_sprite_push_, GET_NUM_ARGS(__VA_ARGS__)))(__VA_ARGS__)


typedef enum ENN_TEXT_ALIGN {
        ENN_LEFT,
        ENN_RIGHT,
        ENN_CENTER
} ENN_TEXT_ALIGN;

typedef enum ENN_TEXT_BOX_BEHAVIOUR {
        ENN_RESIZE_TEXT_WITH_BOX,
        ENN_RESIZE_BOX_WITH_TEXT,
        ENN_WRAP_TEXT_IN_BOX
} ENN_TEXT_BOX_BEHAVIOUR;

typedef struct TextSpecification {
        ENN_TEXT_ALIGN          text_align;
        ENN_TEXT_BOX_BEHAVIOUR  text_box_behaviour;
        f32                     text_size;
        bool                    text_box_border;
} TextSpecification;

ENNDEF_PRIVATE void render_text_push(f32vec2 top_left, f32vec2 bott_right, const char* text, TextSpecification* specification);

