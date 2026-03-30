#version 330 core

in vec2  v_TexCoord;
in vec4  v_Color;
in float v_TexIndex;

uniform sampler2D u_Textures[16];

out vec4 FragColor;

void main() {
    int index = int(v_TexIndex);
    FragColor = texture(u_Textures[index], v_TexCoord) * v_Color;
}