#version 300 es

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODELMATRIX;
uniform mat3 NORMALMATRIX; // inverse-transpose of MODELMATRIX's upper 3x3

out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    vWorldPos = vec3(MODELMATRIX * vec4(aPosition, 1.0));
    // Transform the normal by the normal matrix, not the model matrix --
    // otherwise it stays frozen in the mesh's rest pose while the fan spins.
    vNormal = normalize(NORMALMATRIX * aNormal);
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}
