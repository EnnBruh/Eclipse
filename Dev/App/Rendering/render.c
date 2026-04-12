#include "Rendering/render.h"

struct Renderer global_render;
static Sprite WHITE_TEXTURE;

ENNDEF_PUBLIC void render_init_sprite_sheet(void) {
        DEBUG_TRACE();
        render_image_read(&global_render.sprite_sheet, ENN_APP_DIRECTORY ENN_TEXTURES_PATH "spritesheet.png", !ENN_IMAGE_FLIP);
        glActiveTexture(GL_TEXTURE0);
        global_render.sprite_sheet_id = render_texture_create_image(&global_render.sprite_sheet, ENN_CLAMP_TO_EDGES, ENN_NEAREST);
        stbi_image_free(global_render.sprite_sheet.data);
        glUniform1i(glGetUniformLocation(global_render.shader, "sprite_sheet"), 0);
        DEBUG_UNTRACE();
}

ENNDEF_PUBLIC void render_init_font_atlas(void) {
        DEBUG_TRACE();
        global_render.font_atlas.char_dim = (i32vec2) {
                .x = 8, .y = 16
        };
        global_render.font_atlas.char_per_col = 16;
        global_render.font_atlas.font_dim = (i32vec2) {
                .x = 128, .y = 96
        };
        global_render.font_atlas.font_offset = (i32vec2) {
                .x = 872, .y = 0
        };

        for (char ch = ENN_FONT_ATLAS_FIRST_CHAR; ch <= ENN_FONT_ATLAS_LAST_CHAR; ++ch) {
                i32 row = (i32)(ch - ENN_FONT_ATLAS_FIRST_CHAR) / global_render.font_atlas.char_per_col;
                i32 col = (i32)(ch - ENN_FONT_ATLAS_FIRST_CHAR) % global_render.font_atlas.char_per_col;
                
                global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR] = (f32vec4) {
                        .x = (f32)(col * global_render.font_atlas.char_dim.x + global_render.font_atlas.font_offset.x) / (f32)global_render.sprite_sheet.width,
                        .y = (f32)(row * global_render.font_atlas.char_dim.y + global_render.font_atlas.font_offset.y) / (f32)global_render.sprite_sheet.height,
                        .z = (f32)(col * global_render.font_atlas.char_dim.x + global_render.font_atlas.char_dim.x + global_render.font_atlas.font_offset.x) / (f32)global_render.sprite_sheet.width,
                        .w = (f32)(row * global_render.font_atlas.char_dim.y + global_render.font_atlas.char_dim.y + global_render.font_atlas.font_offset.y) / (f32)global_render.sprite_sheet.height
                };

                // swap(global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].y, global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].w);

                // LOG("ch = '%c' pos = %f %f %f %f", ch,
                //         global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].x,
                //         global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].y,
                //         global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].z,
                //         global_render.font_atlas.char_sprite[ch - ENN_FONT_ATLAS_FIRST_CHAR].w);
        }

        DEBUG_UNTRACE();
}

void render_init(void) {
        DEBUG_TRACE();
        glGenBuffers(1, &global_render.vbo);
        glGenVertexArrays(1, &global_render.vao);

        glBindBuffer(GL_ARRAY_BUFFER, global_render.vbo);
        glBufferData(GL_ARRAY_BUFFER, (sizeof (Vertex)) * ENN_RENDER_VERTEX_BUFF_SIZE, NULL, GL_DYNAMIC_DRAW);

        glBindVertexArray(global_render.vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof (Vertex)), (void *)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, (sizeof (Vertex)), (void *)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof (Vertex)), (void *)offsetof(Vertex, texture_pos));
        glEnableVertexAttribArray(2);

        global_render.shader = render_shader_compile("vertex.glsl", "fragment.glsl");
        ASSERT(global_render.shader != ENN_SHADER_ERR);

        glUseProgram(global_render.shader);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        render_init_sprite_sheet();
        render_init_font_atlas();

        WHITE_TEXTURE = render_sprite_create(&global_render.sprite_sheet,
                (i32vec2) { 0, 0 },
                (i32vec2) { 16, 16 });

        global_render.proj_matrix_location = glGetUniformLocation(global_render.shader, "projection");
        mat4 default_ndc_proj = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
        };
        render_proj_set(default_ndc_proj);

        DEBUG_UNTRACE();
}

void render_term(void) {
        DEBUG_TRACE();
        glDeleteBuffers(1, &global_render.vbo);
        glDeleteVertexArrays(1, &global_render.vao);
        DEBUG_UNTRACE();
}

void render_buff_draw(void) {
        DEBUG_TRACE();
        if (global_render.buff_size > 0) {
                glUseProgram(global_render.shader);
                glBindVertexArray(global_render.vao);
                glBindBuffer(GL_ARRAY_BUFFER, global_render.vbo);

                glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof (Vertex)) * global_render.buff_size, global_render.buff);
                glDrawArrays(GL_TRIANGLES, 0, global_render.buff_size);
                
                // LOG("[Render] Drawing %" PRIi32 " vertices", global_render.buff_size[0]);
                global_render.buff_size = 0;
        }

        DEBUG_UNTRACE();
}

Sprite render_sprite_create(Image* texture, i32vec2 texture_top_left, i32vec2 texture_bott_right) {
        DEBUG_ASSERT(texture != NULL);
        // LOG("%" PRIi32 " %" PRIi32 " %" PRIi32 " %" PRIi32, texture_top_left.x, texture_top_left.y, texture_bott_right.x, texture_bott_right.y);
        // LOG("%" PRIi32  " %" PRIi32, texture -> width, texture -> height);
        return (Sprite) {
                .img = texture,
                .texture_top_left = (f32vec2) {
                        .x = (f32)texture_top_left.x / (f32)texture -> width,
                        .y = (f32)texture_top_left.y / (f32)texture -> height
                },
                .texture_bott_right = (f32vec2) {
                        .x = (f32)texture_bott_right.x / (f32)texture -> width,
                        .y = (f32)texture_bott_right.y / (f32)texture -> height
                }
        };
}

void render_proj_set(const f32* proj_matrix) {
        DEBUG_TRACE();
        render_buff_draw();

        glUseProgram(global_render.shader);
        glUniformMatrix4fv(global_render.proj_matrix_location, 1, GL_TRUE, proj_matrix);
        DEBUG_UNTRACE();
}


void render_rectangle_push(f32vec2 top_left, f32vec2 bott_right, u32 color) {
        DEBUG_TRACE();
        render_sprite_push_color(top_left, bott_right, &WHITE_TEXTURE, color);
        DEBUG_UNTRACE();
}

void render_line_push(f32vec2 pos1, f32vec2 pos2, f32 width, u32 color) {
}

void render_sprite_push_color(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite, u32 color) {
        DEBUG_TRACE();
        if (global_render.buff_size + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                render_buff_draw();

        Vertex* vert = &(global_render.buff[global_render.buff_size]);

        vert[0].pos = top_left;
        vert[0].color = color;
        vert[0].texture_pos = sprite -> texture_top_left;

        vert[1].pos.x = top_left.x;
        vert[1].color = color;
        vert[1].texture_pos.x = sprite -> texture_top_left.x;
        vert[1].pos.y = bott_right.y;
        vert[1].texture_pos.y = sprite -> texture_bott_right.y;

        vert[2].pos.x = bott_right.x;
        vert[2].color = color;
        vert[2].texture_pos.x = sprite -> texture_bott_right.x;
        vert[2].pos.y = top_left.y;
        vert[2].texture_pos.y = sprite -> texture_top_left.y;

        vert[3].pos.x = bott_right.x;
        vert[3].color = color;
        vert[3].texture_pos.x = sprite -> texture_bott_right.x;
        vert[3].pos.y = top_left.y;
        vert[3].texture_pos.y = sprite -> texture_top_left.y;

        vert[4].pos.x = top_left.x;
        vert[4].color = color;
        vert[4].texture_pos.x = sprite -> texture_top_left.x;
        vert[4].pos.y = bott_right.y;
        vert[4].texture_pos.y = sprite -> texture_bott_right.y;

        vert[5].pos = bott_right;
        vert[5].color = color;
        vert[5].texture_pos = sprite -> texture_bott_right;

        global_render.buff_size += 6;
        DEBUG_UNTRACE();
}

void render_sprite_push(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite) {
        DEBUG_TRACE();
        if (global_render.buff_size + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                render_buff_draw();

        Vertex* vert = &(global_render.buff[global_render.buff_size]);

        vert[0].pos = top_left;
        vert[0].color = 0xFFFFFFFF;
        vert[0].texture_pos = sprite -> texture_top_left;

        vert[1].pos.x = top_left.x;
        vert[1].color = 0xFFFFFFFF;
        vert[1].texture_pos.x = sprite -> texture_top_left.x;
        vert[1].pos.y = bott_right.y;
        vert[1].texture_pos.y = sprite -> texture_bott_right.y;

        vert[2].pos.x = bott_right.x;
        vert[2].color = 0xFFFFFFFF;
        vert[2].texture_pos.x = sprite -> texture_bott_right.x;
        vert[2].pos.y = top_left.y;
        vert[2].texture_pos.y = sprite -> texture_top_left.y;

        vert[3].pos.x = bott_right.x;
        vert[3].color = 0xFFFFFFFF;
        vert[3].texture_pos.x = sprite -> texture_bott_right.x;
        vert[3].pos.y = top_left.y;
        vert[3].texture_pos.y = sprite -> texture_top_left.y;

        vert[4].pos.x = top_left.x;
        vert[4].color = 0xFFFFFFFF;
        vert[4].texture_pos.x = sprite -> texture_top_left.x;
        vert[4].pos.y = bott_right.y;
        vert[4].texture_pos.y = sprite -> texture_bott_right.y;

        vert[5].pos = bott_right;
        vert[5].color = 0xFFFFFFFF;
        vert[5].texture_pos = sprite -> texture_bott_right;

        global_render.buff_size += 6;
        DEBUG_UNTRACE();
}

void render_sprite_flip_vertical(Sprite* sprite) {
        DEBUG_TRACE();
        swap(sprite -> texture_top_left.x, sprite -> texture_bott_right.x);
        DEBUG_UNTRACE();
}

void render_sprite_flip_horizontal(Sprite* sprite) {
        DEBUG_TRACE();
        swap(sprite -> texture_top_left.y, sprite -> texture_bott_right.y);
        DEBUG_UNTRACE();
}

void render_text_push(f32vec2 top_left, const char* text, u32 color, f32 text_height) {
        DEBUG_TRACE();
        DEBUG_ASSERT(text != NULL);

        f32 text_width = text_height * ((f32)global_render.font_atlas.char_dim.x / (f32)global_render.font_atlas.char_dim.y);

        f32vec2 cursor = top_left;

        for (i32 i = 0; text[i]; ++i) {
                if (text[i] < ENN_FONT_ATLAS_FIRST_CHAR || text[i] > ENN_FONT_ATLAS_LAST_CHAR) continue ;

                if (text[i] == '\n') {
                        cursor.y += text_height; cursor.x = top_left.x;
                        continue ;
                }

                if (global_render.buff_size + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                        render_buff_draw();

                Vertex *vert = &(global_render.buff[global_render.buff_size]);

                // vert[0].pos = cursor;
                // vert[0].color = color;
                // vert[0].texture_pos = WHITE_TEXTURE;

                // vert[1].pos.x = cursor.x;
                // vert[1].color = color;
                // vert[1].pos.y = cursor.y + text_height;
                // vert[1].texture_pos = WHITE_TEXTURE;

                // vert[2].pos.x = cursor.x + text_width;
                // vert[2].color = color;
                // vert[2].pos.y = cursor.y;
                // vert[2].texture_pos = WHITE_TEXTURE;

                // vert[3].pos.x = cursor.x + text_width;
                // vert[3].color = color;
                // vert[3].pos.y = cursor.y;
                // vert[3].texture_pos = WHITE_TEXTURE;

                // vert[4].pos.x = cursor.x;
                // vert[4].color = color;
                // vert[4].pos.y = cursor.y + text_height;
                // vert[4].texture_pos = WHITE_TEXTURE;

                // vert[5].pos.x = cursor.x + text_width;
                // vert[5].pos.y = cursor.y + text_height;
                // vert[5].color = color;
                // vert[5].texture_pos = WHITE_TEXTURE;


                vert[0].pos = cursor;
                vert[0].color = color;
                vert[0].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].x;
                vert[0].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].y;

                vert[1].pos.x = cursor.x;
                vert[1].color = color;
                vert[1].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].x;
                vert[1].pos.y = cursor.y + text_height;
                vert[1].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].w;

                vert[2].pos.x = cursor.x + text_width;
                vert[2].color = color;
                vert[2].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].z;
                vert[2].pos.y = cursor.y;
                vert[2].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].y;

                vert[3].pos.x = cursor.x + text_width;
                vert[3].color = color;
                vert[3].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].z;
                vert[3].pos.y = cursor.y;
                vert[3].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].y;

                vert[4].pos.x = cursor.x;
                vert[4].color = color;
                vert[4].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].x;
                vert[4].pos.y = cursor.y + text_height;
                vert[4].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].w;

                vert[5].pos.x = cursor.x + text_width;
                vert[5].pos.y = cursor.y + text_height;
                vert[5].color = color;
                vert[5].texture_pos.x = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].z;
                vert[5].texture_pos.y = global_render.font_atlas.char_sprite[text[i] - ENN_FONT_ATLAS_FIRST_CHAR].w;

                global_render.buff_size += 6;
                cursor.x += text_width;
        }
        DEBUG_UNTRACE();
}