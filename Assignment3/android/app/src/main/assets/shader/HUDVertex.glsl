#version 300 es

// One shared unit quad (0,0)-(1,0)-(1,1)-(0,1); every button/bar block just
// changes QUADRECT and HUDCOLOR (see SceneHUD.cpp's drawQuad()) instead of
// uploading new geometry per widget.
layout(location = 0) in vec2 aPos;

uniform mat4 HUDPROJECTION; // orthographic: TransformOrtho(0, w, h, 0, -1, 1)
uniform vec4 QUADRECT;      // x, y, w, h -- pixels, top-left origin

void main()
{
    vec2 pixelPos = QUADRECT.xy + aPos * QUADRECT.zw;
    gl_Position = HUDPROJECTION * vec4(pixelPos, 0.0, 1.0);
}
