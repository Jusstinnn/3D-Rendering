/**
 * SceneHUD.cpp
 *
 * Every widget here is the same unit quad (0,0)-(1,0)-(1,1)-(0,1), stretched
 * and positioned per-draw via a QUADRECT uniform (x, y, w, h in pixels) --
 * see HUDVertex.glsl. That keeps this to one VAO for four buttons plus a
 * 20-block speed bar instead of rebuilding geometry every frame.
 */

#include "SceneHUD.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

namespace {
constexpr float kUnitQuad[4 * 2] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};

constexpr float kSpeedColor[3] = {0.25f, 0.50f, 0.80f}; // +SPD / -SPD -- "Vehicle Controls"
constexpr float kZoomColor[3]  = {0.55f, 0.32f, 0.75f}; // +ZOOM / -ZOOM -- "Camera Controls"
constexpr float kGreen[3]      = {0.25f, 0.85f, 0.30f};
constexpr float kGrey[3]       = {0.35f, 0.35f, 0.38f};

constexpr float kButtonW = 110.0f;
constexpr float kButtonH = 40.0f;
constexpr float kMargin  = 24.0f;
constexpr float kRowGap  = 12.0f;

constexpr float kBlockW = 10.0f;
constexpr float kBlockH = 20.0f;
constexpr int   kBlockCount = 20;
} // namespace

SceneHUD::~SceneHUD()
{
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    if (program_) glDeleteProgram(program_);
}

void SceneHUD::InitModel()
{
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kUnitQuad), kUnitQuad, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
    glBindVertexArray(0);

#ifdef PLATFORM_ANDROID
    program_ = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program_ = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
#endif
    locProjection_ = glGetUniformLocation(program_, "HUDPROJECTION");
    locRect_       = glGetUniformLocation(program_, "QUADRECT");
    locColor_      = glGetUniformLocation(program_, "HUDCOLOR");

    transform.TransformInit();
}

void SceneHUD::Resize(int w, int h)
{
    screenW_ = w;
    screenH_ = h;

    // Top-left origin, Y growing downward -- matches mouse/touch coordinates
    // directly, so TouchDown() below never needs to unproject anything.
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);
    hudProjection_ = *transform.TransformGetProjectionMatrix();

    // Re-derive every widget's pixel rectangle, same idea as re-deriving the
    // perspective projection on resize in Fan/Scene3D.
    btnSpeedUp_   = { kMargin,                       kMargin,                     kButtonW, kButtonH };
    btnZoomIn_    = { kMargin + kButtonW + kRowGap,   kMargin,                     kButtonW, kButtonH };
    btnSpeedDown_ = { kMargin,                       kMargin + kButtonH + kRowGap, kButtonW, kButtonH };
    btnZoomOut_   = { kMargin + kButtonW + kRowGap,   kMargin + kButtonH + kRowGap, kButtonW, kButtonH };

    speedBar_ = { kMargin, static_cast<float>(h) - kBlockH - kMargin, kBlockW * kBlockCount, kBlockH };
}

bool SceneHUD::hitTest(const Rect& r, float x, float y) const
{
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

bool SceneHUD::TouchDown(float x, float y)
{
    if (hitTest(btnSpeedUp_, x, y)) {
        speed = std::min(20.0f, speed + 2.0f);
        activeButton = 0;
        LOGI("+SPD pressed (speed=%.1f)", speed);
        return true;
    }
    if (hitTest(btnSpeedDown_, x, y)) {
        speed = std::max(0.0f, speed - 2.0f);
        activeButton = 1;
        LOGI("-SPD pressed (speed=%.1f)", speed);
        return true;
    }
    if (hitTest(btnZoomIn_, x, y)) {
        zoom = std::max(3.0f, zoom - 1.0f);
        activeButton = 2;
        LOGI("+ZOOM pressed (zoom=%.1f)", zoom);
        return true;
    }
    if (hitTest(btnZoomOut_, x, y)) {
        zoom = std::min(20.0f, zoom + 1.0f);
        activeButton = 3;
        LOGI("-ZOOM pressed (zoom=%.1f)", zoom);
        return true;
    }
    return false;
}

void SceneHUD::TouchRelease()
{
    activeButton = -1;
}

void SceneHUD::drawQuad(const Rect& rect, const float color[3])
{
    glUniform4f(locRect_, rect.x, rect.y, rect.w, rect.h);
    glUniform3fv(locColor_, 1, color);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void SceneHUD::Render()
{
    glUseProgram(program_);
    glUniformMatrix4fv(locProjection_, 1, GL_FALSE, glm::value_ptr(hudProjection_));
    glBindVertexArray(vao_);

    drawQuad(btnSpeedUp_,   kSpeedColor);
    drawQuad(btnSpeedDown_, kSpeedColor);
    drawQuad(btnZoomIn_,    kZoomColor);
    drawQuad(btnZoomOut_,   kZoomColor);

    // 20-block speed bar (Part 6) -- rebuilt from live state every frame,
    // not a static image: green = active, grey = inactive.
    int lit = std::clamp(static_cast<int>(std::round(speed)), 0, kBlockCount);
    for (int i = 0; i < kBlockCount; ++i) {
        Rect block{ speedBar_.x + i * kBlockW, speedBar_.y, kBlockW, kBlockH };
        drawQuad(block, (i < lit) ? kGreen : kGrey);
    }

    glBindVertexArray(0);
}
