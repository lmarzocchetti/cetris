#version 330 core

in vec4 fragColor;
in vec2 fragTexCoord;
out vec4 finalColor;

uniform float time;

void main() {
    vec2 uv = fragTexCoord;

    // Grid mapping
    vec2 grid = uv * 20.0;
    vec2 cell = fract(grid);

    // Glow shape
    float distToCenter = length(cell - 0.5);
    float pulse = sin(time * 4.0 + floor(grid.x) + floor(grid.y)) * 0.5 + 0.5;
    float glow = smoothstep(0.15, 0.0, distToCenter) * pulse;

    // Sparkle shimmer
    float sparkle = sin((grid.x + grid.y) * 10.0 + time * 5.0) * 0.1;

    // Use the original color as the glow base
    vec3 baseColor = fragColor.rgb;
    vec3 color = baseColor * (glow + sparkle + 0.2); // add brightness

    // Slight mix with original color to keep it coherent
    color = mix(baseColor, color, 0.9);

    finalColor = vec4(color, fragColor.a);
}
