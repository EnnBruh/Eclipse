#include "Rendering/render.h"

struct Renderer global_render;

void render_init(void) {
        DEBUG_TRACE();
        glGenBuffers(1, &global_render.vbo);
        glGenVertexArrays(1, &global_render.vao);

        glBindBuffer(GL_ARRAY_BUFFER, global_render.vbo);
        glBufferData(GL_ARRAY_BUFFER, (sizeof(Vertex)) * ENN_RENDER_VERTEX_BUFF_SIZE, NULL, GL_DYNAMIC_DRAW);

        glBindVertexArray(global_render.vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(Vertex)), (void *)offsetof(Vertex, pos));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, (sizeof(Vertex)), (void *)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (sizeof(Vertex)), (void *)offsetof(Vertex, texture_pos));
        glEnableVertexAttribArray(2);

        global_render.shader = render_shader_compile("vertex.glsl", "fragment.glsl");
        ASSERT(global_render.shader != ENN_SHADER_ERR);

        glUseProgram(global_render.shader);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        render_image_read(&global_render.sprite_sheet, ENN_APP_DIRECTORY ENN_TEXTURES_PATH "spritesheet.png", false);
        render_image_read(&global_render.font_atlas, ENN_APP_DIRECTORY ENN_TEXTURES_PATH "font_atlas.png", false);

        global_render.sprite_sheet_id = render_texture_create_image(&global_render.sprite_sheet, ENN_CLAMP_TO_EDGES, ENN_NEAREST);
        global_render.font_atlas_id   = render_texture_create_image(&global_render.font_atlas, ENN_CLAMP_TO_EDGES, ENN_NEAREST);

        stbi_image_free(global_render.sprite_sheet.data);
        stbi_image_free(global_render.font_atlas.data);

        glUniform1i(glGetUniformLocation(global_render.shader, "sprite_sheet"), 0);
        glUniform1i(glGetUniformLocation(global_render.shader, "font_atlas"), 1);

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
        if (global_render.buff_size[0] > 0) {
                glUseProgram(global_render.shader);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, global_render.sprite_sheet_id);
                glBindVertexArray(global_render.vao);
                glBindBuffer(GL_ARRAY_BUFFER, global_render.vbo);

                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * global_render.buff_size[0], global_render.buff[0]);
                glDrawArrays(GL_TRIANGLES, 0, global_render.buff_size[0]);
                
                // LOG("[Render] Drawing %" PRIi32 " vertices", global_render.buff_size[0]);
                global_render.buff_size[0] = 0;
        }

        if (global_render.buff_size[1] > 0) {
                glUseProgram(global_render.shader);
                glBindTexture(GL_TEXTURE_2D, global_render.font_atlas_id);
                glBindVertexArray(global_render.vao);
                glBindBuffer(GL_ARRAY_BUFFER, global_render.vbo);

                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * global_render.buff_size[1], global_render.buff[1]);
                glDrawArrays(GL_TRIANGLES, 0, global_render.buff_size[1]);
                
                global_render.buff_size[1] = 0;
        }
        DEBUG_UNTRACE();
}

void render_proj_set(const f32* proj_matrix) {
        DEBUG_TRACE();
        if (global_render.buff_size[0] > 0 || global_render.buff_size[1] > 0) 
                render_buff_draw();
        
        glUseProgram(global_render.shader);
        glUniformMatrix4fv(global_render.proj_matrix_location, 1, GL_TRUE, proj_matrix);
        DEBUG_UNTRACE();
}

static f32vec2 WHITE_TEXTURE = { 0.0, 0.0 };

void render_rectangle_push(f32vec2 top_left, f32vec2 bott_right, u32 color) {
        DEBUG_TRACE();
        if (global_render.buff_size[0] + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                render_buff_draw();

        Vertex* vert = &(global_render.buff[0][global_render.buff_size[0]]);

        vert[0].pos = top_left; vert[0].color = color; vert[0].texture_pos = WHITE_TEXTURE;
        vert[1].pos = (f32vec2) { .x = top_left.x, .y = bott_right.y }; vert[1].color = color; vert[1].texture_pos = WHITE_TEXTURE;
        vert[2].pos = (f32vec2) { .x = bott_right.x, .y = top_left.y }; vert[2].color = color; vert[2].texture_pos = WHITE_TEXTURE;
        vert[3].pos = (f32vec2) { .x = bott_right.x, .y = top_left.y }; vert[3].color = color; vert[3].texture_pos = WHITE_TEXTURE;
        vert[4].pos = (f32vec2) { .x = top_left.x, .y = bott_right.y }; vert[4].color = color; vert[4].texture_pos = WHITE_TEXTURE;
        vert[5].pos = bott_right; vert[5].color = color; vert[5].texture_pos = WHITE_TEXTURE;
        global_render.buff_size[0] += 6;
        DEBUG_UNTRACE();
}

void render_sprite_push_color(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite, u32 color) {
        DEBUG_TRACE();
        if (global_render.buff_size[0] + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                render_buff_draw();

        Vertex* vert = &(global_render.buff[0][global_render.buff_size[0]]);

        f32vec2 tex_top_left = {
                .x = sprite -> texture_top_left.x / sprite -> img -> width,
                .y = sprite -> texture_top_left.y / sprite -> img -> height
        };

        f32vec2 tex_bott_right = {
                .x = sprite -> texture_bott_right.x / sprite -> img -> width,
                .y = sprite -> texture_bott_right.y / sprite -> img -> height
        };

        vert[0].pos = top_left; vert[0].color = color; vert[0].texture_pos = tex_top_left;

        vert[1].pos.x = top_left.x; vert[1].color = color; vert[1].texture_pos.x = tex_top_left.x;
        vert[1].pos.y = bott_right.y;                         ; vert[1].texture_pos.y = tex_bott_right.y;

        vert[2].pos.x = bott_right.x; vert[2].color = color; vert[2].texture_pos.x = tex_bott_right.x;
        vert[2].pos.y = top_left.y;                               vert[2].texture_pos.y = tex_top_left.y;

        vert[3].pos.x = bott_right.x; vert[3].color = color; vert[3].texture_pos.x = tex_bott_right.x;
        vert[3].pos.y = top_left.y;                               vert[3].texture_pos.y = tex_top_left.y;

        vert[4].pos.x = top_left.x; vert[4].color = color; vert[4].texture_pos.x = tex_top_left.x;
        vert[4].pos.y = bott_right.y;                           vert[4].texture_pos.y = tex_bott_right.y;

        vert[5].pos = bott_right; vert[5].color = color; vert[5].texture_pos = tex_bott_right;

        global_render.buff_size[0] += 6;
        DEBUG_UNTRACE();
}

void render_sprite_push(f32vec2 top_left, f32vec2 bott_right, Sprite* sprite) {
        DEBUG_TRACE();
        if (global_render.buff_size[0] + 6 > ENN_RENDER_VERTEX_BUFF_SIZE)
                render_buff_draw();

        Vertex* vert = &(global_render.buff[0][global_render.buff_size[0]]);

        f32vec2 tex_top_left = {
                .x = sprite -> texture_top_left.x / sprite -> img -> width,
                .y = sprite -> texture_top_left.y / sprite -> img -> height
        };

        f32vec2 tex_bott_right = {
                .x = sprite -> texture_bott_right.x / sprite -> img -> width,
                .y = sprite -> texture_bott_right.y / sprite -> img -> height
        };

        vert[0].pos = top_left; vert[0].color = 0xFFFFFFFF; vert[0].texture_pos = tex_top_left;

        vert[1].pos.x = top_left.x; vert[1].color = 0xFFFFFFFF; vert[1].texture_pos.x = tex_top_left.x;
        vert[1].pos.y = bott_right.y;                         ; vert[1].texture_pos.y = tex_bott_right.y;

        vert[2].pos.x = bott_right.x; vert[2].color = 0xFFFFFFFF; vert[2].texture_pos.x = tex_bott_right.x;
        vert[2].pos.y = top_left.y;                               vert[2].texture_pos.y = tex_top_left.y;

        vert[3].pos.x = bott_right.x; vert[3].color = 0xFFFFFFFF; vert[3].texture_pos.x = tex_bott_right.x;
        vert[3].pos.y = top_left.y;                               vert[3].texture_pos.y = tex_top_left.y;

        vert[4].pos.x = top_left.x; vert[4].color = 0xFFFFFFFF; vert[4].texture_pos.x = tex_top_left.x;
        vert[4].pos.y = bott_right.y;                           vert[4].texture_pos.y = tex_bott_right.y;

        vert[5].pos = bott_right; vert[5].color = 0xFFFFFFFF; vert[5].texture_pos = tex_bott_right;

        global_render.buff_size[0] += 6;
        DEBUG_UNTRACE();
}