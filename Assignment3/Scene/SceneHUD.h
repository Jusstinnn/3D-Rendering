#pragma once

/**
 * SceneHUD.h
 *
 * New in Quiz 3 (Parts 4, 5, 6, 7). Four buttons (+SPD, -SPD, +ZOOM, -ZOOM)
 * and a 20-block speed bar, all drawn in one orthographic screen-space pass
 * -- pixels map 1:1 to clip space, no camera, no depth. Owns no 3D state
 * itself; it just tracks speed/zoom and Renderer::render() reads those
 * values back into Scene3D every frame.
 *
 * Bonus: press-and-hold. TouchDown() applies a button's action once
 * immediately (so a quick tap still feels instant), then updateHold() --
 * called once per frame from Render() -- keeps re-applying it at a fixed
 * rate for as long as TouchRelease() hasn't fired, after a short initial
 * delay (the same "typematic" pattern OS keyboards use for held keys, not
 * a literal every-frame repeat -- see SceneHUD.cpp for why).
 */

#include "Model.h"
#include "Transform.h"
#include "Platform.h"

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    SceneHUD() = default;
#endif

    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    bool TouchDown(float x, float y); // HUD hit-testing (Part 5). Returns true if a button was hit.
    void TouchRelease();

    float CurrentSpeed() const { return speed; }
    float CurrentZoom() const { return zoom; }

private:
    struct Rect { float x, y, w, h; };

    void drawQuad(const Rect& rect, const float color[3]);
    bool hitTest(const Rect& rect, float x, float y) const;
    void applyButtonAction(int button); // 0=+SPD, 1=-SPD, 2=+ZOOM, 3=-ZOOM
    void updateHold();                  // called once per frame from Render()

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint program_ = 0;
    GLint  locProjection_ = -1;
    GLint  locRect_       = -1;
    GLint  locColor_      = -1;

    Transform transform;
    glm::mat4 hudProjection_ { 1.0f };

    int screenW_ = 800;
    int screenH_ = 600;

    Rect btnSpeedUp_{}, btnSpeedDown_{}, btnZoomIn_{}, btnZoomOut_{};
    Rect speedBar_{}; // top-left corner of the 20-block bar; each block is 10x20

    float speed = 8.0f;  // 0..20 -- Part 6
    float zoom  = 8.0f;  // camera distance, clamped [3, 20] in Scene3D -- Part 7
    int   activeButton = -1; // which button (if any) is held
    int   holdFrameCounter = 0;
    static constexpr int kHoldInitialDelayFrames  = 20; // ~0.33s before repeat kicks in
    static constexpr int kHoldRepeatIntervalFrames = 6; // ~0.1s between repeats once active

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
