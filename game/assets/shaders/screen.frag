#version 330 core
in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Scene;
uniform sampler2D u_Lights;
uniform float     u_Exposure   = 1.0;
uniform float     u_Vignette   = 0.4;
uniform float     u_Saturation = 1.0;
uniform bool      u_UseLights  = false;

vec3 applyVignette(vec3 color, vec2 uv) {
    vec2  d = uv - 0.5;
    float v = 1.0 - dot(d, d) * 2.5 * u_Vignette;
    return color * clamp(v, 0.0, 1.0);
}

vec3 applySaturation(vec3 color, float sat) {
    float lum = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(lum), color, sat);
}

void main() {
    vec3 scene = texture(u_Scene, v_TexCoord).rgb;

    // multiply by light map if lighting enabled
    if (u_UseLights) {
        vec3 light = texture(u_Lights, v_TexCoord).rgb;
        scene *= light;
    }

    scene  = applySaturation(scene, u_Saturation);
    scene  = applyVignette  (scene, v_TexCoord);
    scene *= u_Exposure;

    // simple tone mapping
    scene = scene / (scene + vec3(1.0));
    FragColor = vec4(scene, 1.0);
}