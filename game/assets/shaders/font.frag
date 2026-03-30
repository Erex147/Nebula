#version 330 core

in vec2  v_TexCoord;
in vec4  v_Color;
in float v_TexIndex;

uniform sampler2D u_Textures[16];

out vec4 FragColor;

void main() {
    int   idx   = int(v_TexIndex);
    float alpha = texture(u_Textures[idx], v_TexCoord).r;
    FragColor   = vec4(v_Color.rgb, v_Color.a * alpha);
}