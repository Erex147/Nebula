#version 330 core
in vec2 v_TexCoord;
in vec4 v_Color;
out vec4 FragColor;

void main() {
    // radial falloff: bright at centre, zero at edges
    vec2  uv   = v_TexCoord * 2.0 - 1.0; // -1 to 1
    float dist = length(uv);
    float atten = clamp(1.0 - dist, 0.0, 1.0);
    atten = atten * atten * (3.0 - 2.0 * atten); // smoothstep
    FragColor = vec4(v_Color.rgb * atten * v_Color.a, 1.0);
}