#version 300 es
precision mediump float;

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOS; // world space
uniform vec3 VIEWPOS;  // world-space camera position, for the specular term

out vec4 fragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(LIGHTPOS - vWorldPos);
    vec3 V = normalize(VIEWPOS  - vWorldPos);
    vec3 R = reflect(-L, N);

    vec3 ambient  = 0.20 * PARTCOLOR;
    float diff    = max(dot(N, L), 0.0);
    vec3 diffuse  = diff * PARTCOLOR;
    float spec    = pow(max(dot(V, R), 0.0), 32.0);
    vec3 specular = spec * vec3(0.6);

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
