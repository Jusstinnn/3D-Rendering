/**
 * Fan.cpp
 *
 * See Fan.h for the part hierarchy and gesture behaviour. This file:
 *   Part 1 - uploads the single 8-vertex/36-index cube once (VBO with two
 *            sub-regions: position, then per-corner shade + one IBO), records
 *            it into one VAO, and draws it 7 times through the Transform
 *            push/pop stack, tinting each part via the PARTCOLOR uniform.
 *   Part 2 - advances spinAngle every frame the fan is on.
 *   Part 3 - TouchEventDown/Move/Release distinguish a tap from a swipe via
 *            movedDistance_ vs kTapThreshold, and toggle fanOn on a tap.
 *   Part 4 - TouchEventMove measures drag velocity with std::chrono and
 *            converts it into a clamped dragBoost; release zeroes it.
 */

#include "Fan.h"
#include "ShaderHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

namespace {

// 8 shared corners of a unit cube, centred at the origin.
const float kPositions[8 * 3] = {
    -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f, // back  (0-3)
    -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f, // front (4-7)
};

// Grayscale factor per corner: front corners brighter than back corners.
const float kShade[8] = {
    0.6f, 0.6f, 0.6f, 0.6f,   // back  (0-3)
    1.0f, 1.0f, 1.0f, 1.0f,   // front (4-7)
};

// 6 faces x 2 triangles x 3 indices = 36. No culling is enabled, so winding
// order doesn't matter for correctness, only that every face is covered.
const GLushort kIndices[36] = {
    4, 5, 6,  4, 6, 7,   // front
    1, 0, 3,  1, 3, 2,   // back
    0, 4, 7,  0, 7, 3,   // left
    5, 1, 2,  5, 2, 6,   // right
    7, 6, 2,  7, 2, 3,   // top
    0, 1, 5,  0, 5, 4,   // bottom
};

constexpr float kBrown[3]     = {0.45f, 0.28f, 0.12f};
constexpr float kLightGray[3] = {0.55f, 0.55f, 0.58f};
constexpr float kDarkGray[3]  = {0.20f, 0.20f, 0.22f};
constexpr float kBladeColors[4][3] = {
    {1.0f, 0.0f, 0.0f},  // 0: red
    {0.0f, 0.0f, 1.0f},  // 1: blue
    {1.0f, 0.55f, 0.0f}, // 2: orange
    {0.0f, 1.0f, 0.0f},  // 3: green
};

} // namespace

Fan::~Fan()
{
    if (ibo_)     glDeleteBuffers(1, &ibo_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    if (program_) glDeleteProgram(program_);
}

void Fan::InitModel()
{
    // --- one VBO: positions then shades, uploaded as two sub-regions ------
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions) + sizeof(kShade), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,                 sizeof(kPositions), kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(kPositions), sizeof(kShade),    kShade);

    // --- one IBO ------------------------------------------------------------
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    // --- one VAO capturing both attribute pointers + the IBO binding -------
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0); // layout(location = 0) position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1); // layout(location = 1) shade
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(sizeof(kPositions)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    glBindVertexArray(0);

    // --- shader program ------------------------------------------------------
#ifdef PLATFORM_ANDROID
    program_ = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program_ = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif
    locMVP_       = glGetUniformLocation(program_, "MODELVIEWPROJECTIONMATRIX");
    locPartColor_ = glGetUniformLocation(program_, "PARTCOLOR");

    glEnable(GL_DEPTH_TEST);

    transform.TransformInit();
}

void Fan::Resize(int w, int h)
{
    if (h <= 0) h = 1;
    glViewport(0, 0, w, h);

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

void Fan::drawPart(const float color[3])
{
    glUseProgram(program_);

    glm::mat4 mvp = *transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(locMVP_, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3fv(locPartColor_, 1, color);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(0));
    glBindVertexArray(0);
}

void Fan::Render()
{
    // ---- Part 2: advance the blade angle -----------------------------------
    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- Part 1: the scene graph, exactly per the assignment pseudocode ---
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // world: pull-back + 3D tilt, shared by every part
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // base
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(kBrown);
    transform.TransformPopMatrix();

    // pole
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(kLightGray);
    transform.TransformPopMatrix();

    // hub
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.0f);
        transform.TransformScale(0.3f, 0.3f, 0.3f);
        drawPart(kDarkGray);
    transform.TransformPopMatrix();

    // 4 blades, 90 degrees apart, spinning around the hub's Z axis
    for (int i = 0; i < 4; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.8f, 0.05f);
            drawPart(kBladeColors[i]);
        transform.TransformPopMatrix();
    }
}

// ---- Part 3 & 4: gesture pipeline ------------------------------------------

void Fan::TouchEventDown(float x, float y)
{
    lastX_ = x;
    lastY_ = y;
    movedDistance_ = 0.0f;
    lastMoveTime_ = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    float dx = x - lastX_;
    float dy = y - lastY_;
    float dist = std::sqrt(dx * dx + dy * dy);
    movedDistance_ += dist;

    auto now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime_).count();
    lastMoveTime_ = now;

    if (dtMs > 0.0f) {
        float velocity = dist / dtMs;   // pixels per millisecond
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
        LOGI("Fan drag velocity=%.3f px/ms -> boost=%.2f", velocity, dragBoost);
    }

    lastX_ = x;
    lastY_ = y;
}

void Fan::TouchEventRelease(float /*x*/, float /*y*/)
{
    if (movedDistance_ < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
    dragBoost = 0.0f;   // boost only applies while dragging
}
