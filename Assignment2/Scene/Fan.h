#pragma once

/**
 * Fan.h
 *
 * Assignment 2 — "3D Fan"
 *
 * Part hierarchy (one cube, drawn 7 times with a different model matrix,
 * composed on the Transform push/pop stack — see Transform.h header comment
 * for the 2D windmill this pattern is based on):
 *
 *   [B3]
 *    |
 *   [B2]--(hub)--[B0]   blades rotate around the hub's Z axis, 90 degrees
 *    |                  apart, driven by spinAngle
 *   [B1]
 *    |
 *   pole                connects hub to base
 *    |
 *  __base__              sits on the "table"
 *
 * Gesture behaviour:
 *   TAP   (down+release, little movement) -> toggle fanOn, log Fan ON/OFF
 *   SWIPE (drag)                          -> spins faster proportional to
 *                                             drag velocity (dragBoost);
 *                                             releases back to base speed
 */

#include "Model.h"
#include "Transform.h"
#include "Platform.h"

#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Fan() = default;
#endif

    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y) override;
    void TouchEventMove(float x, float y) override;
    void TouchEventRelease(float x, float y) override;

private:
    void drawPart(const float color[3]);

    // GL objects for the single shared 8-vertex / 36-index cube.
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ibo_ = 0;
    GLuint program_ = 0;
    GLint  locMVP_ = -1;
    GLint  locPartColor_ = -1;

    Transform transform;

    float spinAngle  = 0.0f;   // current blade angle (degrees)
    bool  fanOn      = true;   // toggled by a tap
    float dragBoost  = 0.0f;   // extra speed while dragging, 0 otherwise

    static constexpr float kBaseSpeed    = 1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;  // clamp for the drag boost
    static constexpr float kBoostScale   = 8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;  // px: below this it's a tap

    float lastX_ = 0.0f, lastY_ = 0.0f;   // previous touch position
    float movedDistance_ = 0.0f;          // total movement since touch-down
    std::chrono::steady_clock::time_point lastMoveTime_;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
