#pragma once

/**
 * Fan.h
 *
 * Android Programming Quiz 3 -- "Fan Dashboard HUD"
 *
 * Sunburst redesign: a gray square base, a cylindrical pole, a spherical
 * hub, and kBladeCount thin rectangular-box blades radiating from the hub
 * (real 3D boxes, same as the original Quiz 2 blades, so you see a front,
 * back, and side face as each one rotates -- not a flat card) -- still
 * drawn through the exact same Transform push/pop hierarchy as the
 * original Quiz 2 cube-based fan (only the mesh bound per part changed;
 * every translate/scale number is unchanged and still describes a part
 * that fits the local [-0.5, 0.5]^3 box, which is also what Scene3D's
 * picking ray test assumes).
 *
 *   [Blade 15]
 *       |
 *   [Blade 1]--(hub, sphere)--[Blade 0]   blades pinwheel-spin around the
 *       |                                  hub's local Z axis (spinAngle)
 *   pole (cylinder)
 *       |
 *   base (square slab)
 *
 * The whole assembly (base + pole + hub + blades together) also turns as
 * one rigid body around the world-up Y axis (assemblyAngle) -- a slow
 * "turntable" spin layered underneath the blades' own faster pinwheel spin.
 *
 * What changed from Quiz 2:
 *   - TouchEventDown/Move/Release and the tap-to-toggle / drag-to-boost
 *     fields are GONE. Fan no longer reads input directly -- Scene3D feeds
 *     it a speed value (from SceneHUD's +SPD/-SPD buttons) every frame via
 *     SetSpeed(), and a camera view/projection via SetViewProjection().
 *   - Every part carries a real vertex normal (not a baked shade factor)
 *     so Phong shading has something to light against.
 *   - Every part's model matrix is cached each frame (partModelMatrices_)
 *     so Scene3D::PickAt() can ray-test against exactly the geometry that
 *     was just drawn, and ToggleSelected() flips that part's highlight
 *     independently -- multiple parts can be highlighted at once, each
 *     toggled on/off by its own clicks, instead of one click stealing the
 *     highlight from whatever was selected before.
 */

#include "Model.h"
#include "Transform.h"
#include "Platform.h"

#include <array>
#include <vector>
#include <glm/glm.hpp>

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

    // ---- Driven by Scene3D every frame (Parts 2, 6, 7) ---------------------
    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLightPos(const glm::vec3& lightPos);
    void SetSpeed(float speed); // 0..20, straight from SceneHUD's speed bar

    // ---- Picking, used by Scene3D::PickAt() (Part 8) -----------------------
    static constexpr int kBladeCount = 16;
    static constexpr int kPartCount  = 3 + kBladeCount; // base, pole, hub, then blades

    int GetPartCount() const { return kPartCount; }
    glm::mat4 GetPartMatrix(int index) const { return partModelMatrices_[index]; }
    const char* GetPartName(int index) const;

    // Multi-select: each part's highlight toggles independently.
    void ToggleSelected(int index);
    void ClearSelection();
    bool IsSelected(int index) const;

private:
    // One VAO/VBO/IBO per distinct mesh shape, each built to fit inside a
    // local [-0.5, 0.5]^3 box -- so every part's Transform push/pop
    // translate+scale numbers work identically no matter which shape is
    // bound, and Scene3D's OBB picking test stays valid for all of them.
    struct MeshBuffers {
        GLuint vao = 0, vbo = 0, ibo = 0;
        GLsizei indexCount = 0;
    };

    void buildMesh(MeshBuffers& mesh,
                    const std::vector<float>& positions,
                    const std::vector<float>& normals,
                    const std::vector<GLushort>& indices);
    void destroyMesh(MeshBuffers& mesh);
    void drawPart(int partIndex, const MeshBuffers& mesh, const float color[3]);

    MeshBuffers meshCube_;     // base + every blade (real 3D boxes, shared mesh)
    MeshBuffers meshCylinder_; // pole
    MeshBuffers meshSphere_;   // hub

    GLuint program_ = 0;
    GLint  locMVP_          = -1;
    GLint  locModel_        = -1;
    GLint  locNormalMatrix_ = -1;
    GLint  locPartColor_    = -1;
    GLint  locLightPos_     = -1;
    GLint  locViewPos_      = -1;

    Transform transform;

    float spinAngle     = 0.0f; // blade pinwheel spin, around local Z (relative to hub)
    float assemblyAngle = 0.0f; // whole-fan turntable spin, around world Y (base visibly turns)
    float speed_ = 8.0f;        // 0..20, set by Scene3D::SetSpeed (from SceneHUD)
    static constexpr float kSpeedToBladeDegPerFrame    = 0.15f; // blade pinwheel rate
    static constexpr float kSpeedToAssemblyDegPerFrame = 0.04f; // turntable rate (much slower, layered underneath)

    glm::mat4 viewMatrix_ { 1.0f };
    glm::mat4 projMatrix_ { 1.0f };
    glm::vec3 viewPos_    { 0.0f, 0.0f, 0.0f }; // world-space camera position, for specular
    glm::vec3 lightPos_   { 6.0f, 10.0f, 6.0f };

    std::array<glm::mat4, kPartCount> partModelMatrices_{};
    std::array<bool, kPartCount> selectedParts_{}; // each part's highlight, independent

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
