#version 300 es
precision mediump float;

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 LIGHTPOS;
uniform vec3 VIEWPOS;
uniform float TILESIZE;
uniform vec3 COLOR_A;
uniform vec3 COLOR_B;

out vec4 fragColor;

// No texture: alternate colour per grid cell, purely from world-space (x, z).
float checker(vec2 worldXZ, float tileSize)
{
    vec2 cell = floor(worldXZ / tileSize);
    float parity = mod(cell.x + cell.y, 2.0);
    return parity; // 0.0 or 1.0 -> mix() between two colours
}

void main()
{
    float p = checker(vWorldPos.xz, TILESIZE);
    vec3 materialColor = mix(COLOR_A, COLOR_B, p);

    // Same Phong formula as the fan (FanFragment.glsl) -- the checkerboard
    // pattern feeds in as the material colour instead of PARTCOLOR.
    vec3 N = normalize(vNormal);
    vec3 L = normalize(LIGHTPOS - vWorldPos);
    vec3 V = normalize(VIEWPOS  - vWorldPos);
    vec3 R = reflect(-L, N);

    vec3 ambient  = 0.20 * materialColor;
    float diff    = max(dot(N, L), 0.0);
    vec3 diffuse  = diff * materialColor;
    float spec    = pow(max(dot(V, R), 0.0), 32.0);
    vec3 specular = spec * vec3(0.4);

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
