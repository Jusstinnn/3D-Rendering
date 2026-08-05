#pragma once

/**
 * Scene3D.h
 *
 * New in Quiz 3 (Parts 1, 2, 7, 8). Owns the Fan (from Quiz 2) and the new
 * Ground, builds the shared perspective camera + Phong light once per
 * frame and hands it to both, and owns PickAt() -- the unproject-a-click
 * -into-a-ray-and-test-each-part technique from the Scene Graph examples
 * (1-SceneGraph-RobotArm-Modern / 2-SceneGraph-Windmill-Modern).
 */

#include "Model.h"

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan;
class Ground;

class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* mgr);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d); // called with SceneHUD's zoom value (Part 7)
    void SetSpeed(float speed);      // called with SceneHUD's speed value (Part 6)

    bool PickAt(float screenX, float screenY); // 3D picking (Part 8)

private:
    glm::mat4 BuildViewMatrix() const;
    glm::mat4 BuildProjectionMatrix() const;

    Fan* fan = nullptr;
    Ground* ground = nullptr;

    float cameraDistance = 8.0f; // clamped to [3, 20] in SetCameraDistance
    int   viewportW_ = 800;
    int   viewportH_ = 600;
    float aspect_ = 800.0f / 600.0f;

    glm::vec3 lightPos_ { 6.0f, 10.0f, 6.0f }; // shared by Fan + Ground
};
