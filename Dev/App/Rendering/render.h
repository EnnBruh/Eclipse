#ifndef ENN_CUSTOM_RENDER_H
#define ENN_CUSTOM_RENDER_H

#include "core.h"

typedef struct Vertex {
        f32vec2         pos;
        u32             color;
        f32vec2         texture_pos;
} Vertex;


#define ENN_RENDER_VERTEX_BUFF_SIZE 16384

typedef GLint UniformLocation;

#define ENN_FONT_ATLAS_FIRST_CHAR ' '
#define ENN_FONT_ATLAS_LAST_CHAR '~'
extern struct Renderer {
        Vertex          buff[2][ENN_RENDER_VERTEX_BUFF_SIZE];
        i32             buff_size[2];

        VAOID           vao;
        VBOID           vbo;
        ShaderID        shader;

        UniformLocation proj_matrix_location;

        TextureID       sprite_sheet_id;
        TextureID       font_atlas_id;

        Image           sprite_sheet;
        Image           font_atlas;


        i32vec2         font_char_dim;
        f32vec4         char_texture_pos[ENN_FONT_ATLAS_LAST_CHAR - ENN_FONT_ATLAS_FIRST_CHAR];
} global_render;

typedef struct Sprite {
        Image*  img;
        f32vec2 texture_top_left;
        f32vec2 texture_bott_right;
} Sprite;

ENNDEF_PRIVATE void render_init(void);
ENNDEF_PRIVATE void render_term(void);
ENNDEF_PRIVATE void render_buff_draw(void);

ENNDEF_PRIVATE Sprite render_sprite_create(Image* texture, i32vec2 texture_top_left, i32vec2 texture_bott_right);

typedef f32 mat4[16];
typedef f32 mat3[9];
typedef f32 mat2[4];
ENNDEF_PRIVATE void render_proj_set(const f32* proj_matrix);

ENNDEF_PRIVATE void render_rectangle_push(f32vec2 top_left, f32vec2 bott_right, u32 color);
ENNDEF_PRIVATE void render_line_push(f32vec2 pos1, f32vec2 pos2, f32 width, u32 color);
ENNDEF_PRIVATE void render_sprite_push_color(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite, u32 color);
ENNDEF_PRIVATE void render_sprite_push(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite);
ENNDEF_PRIVATE void render_sprite_flip_horizontal(Sprite* sprite);
ENNDEF_PRIVATE void render_sprite_flip_vertical(Sprite* sprite);


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


#endif