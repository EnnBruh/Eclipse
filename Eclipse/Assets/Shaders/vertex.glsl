#version 330

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texture_pos;

out vec4 m_color;
out vec2 m_texture_pos;

uniform mat4 projection;

void main() {
        gl_Position = projection * vec4(pos, 0.0, 1.0);
        m_color = vec4(color.w, color.z, color.y, color.x);
        m_texture_pos = texture_pos;
}