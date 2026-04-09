#version 330

in vec2 m_texture_pos;
in vec4 m_color;

uniform sampler2D sprite_sheet;
uniform sampler2D font_atlas;
uniform bool is_text;

out vec4 f_color;

void main() {
        f_color = m_color * texture(sprite_sheet, m_texture_pos);
}
