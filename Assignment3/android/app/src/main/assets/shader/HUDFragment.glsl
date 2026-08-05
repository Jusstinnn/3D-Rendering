#version 300 es
precision mediump float;

// Flat-coloured screen-space quads -- no lighting, no texture, no fonts.
uniform vec3 HUDCOLOR;

out vec4 fragColor;

void main()
{
    fragColor = vec4(HUDCOLOR, 1.0);
}
