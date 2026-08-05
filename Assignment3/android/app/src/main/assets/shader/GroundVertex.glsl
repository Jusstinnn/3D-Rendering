#version 300 es

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    vWorldPos = vec3(MODELMATRIX * vec4(aPosition, 1.0));
    vNormal = normalize(NORMALMATRIX * aNormal);
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}
