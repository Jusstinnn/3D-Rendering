#pragma once

/**
 * Ground.h
 *
 * New in Quiz 3 (Part 3). A single big quad, Phong-lit like the fan, with a
 * checkerboard pattern computed procedurally in the fragment shader from
 * world-space (x, z) -- no texture, no second VBO shape.
 */

#include "Model.h"
#include "Transform.h"
#include "Platform.h"

#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Ground() = default;
#endif

    ~Ground() override;

    void InitModel() override;
    void Render() override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLightPos(const glm::vec3& lightPos);

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ibo_ = 0;
    GLuint program_ = 0;
    GLint  locMVP_          = -1;
    GLint  locModel_        = -1;
    GLint  locNormalMatrix_ = -1;
    GLint  locLightPos_     = -1;
    GLint  locViewPos_      = -1;
    GLint  locTileSize_     = -1;
    GLint  locColorA_       = -1;
    GLint  locColorB_       = -1;

    Transform transform;

    glm::mat4 viewMatrix_ { 1.0f };
    glm::mat4 projMatrix_ { 1.0f };
    glm::vec3 viewPos_    { 0.0f, 0.0f, 0.0f };
    glm::vec3 lightPos_   { 6.0f, 10.0f, 6.0f };

    // Sits just under the fan's base (base spans roughly y = -2.73..-2.48).
    static constexpr float kGroundY   = -2.8f;
    static constexpr float kHalfSize  = 60.0f; // large enough to read as "infinite"
    static constexpr float kTileSize  = 2.0f;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
