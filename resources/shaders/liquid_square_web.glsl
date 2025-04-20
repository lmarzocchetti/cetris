#version 300 es

precision mediump float;

in vec4 fragColor;
in vec2 fragTexCoord;
out vec4 finalColor;

uniform float time;

void main() {
    vec2 uv = fragTexCoord;

    // Grid coordinates
    vec2 grid = uv * 20.0;
    vec2 cell = fract(grid);

    // Distance from center of cell
    float distToCenter = length(cell - 0.5);

    // Pulse animation
    float pulse = sin(time * 4.0 + floor(grid.x) + floor(grid.y)) * 0.5 + 0.5;

    // Glow shape and sparkle
    float glow = smoothstep(0.15, 0.0, distToCenter) * pulse;
    float sparkle = sin((grid.x + grid.y) * 10.0 + time * 5.0) * 0.1;

    // Use original fragment color
    vec3 baseColor = fragColor.rgb;

    // Final glow color with slight brightness bump
    vec3 color = baseColor * (glow + sparkle + 0.5); // +0.3 = slightly brighter

    // Blend back with original to keep tone
    color = mix(baseColor, color, 0.92); // slightly less mix = brighter effect

    finalColor = vec4(color, fragColor.a);
}
