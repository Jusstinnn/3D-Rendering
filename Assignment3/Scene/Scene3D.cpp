/**
 * Scene3D.cpp
 *
 * Builds one shared view + projection per frame (perspective -- everything
 * in here should recede into the distance, unlike SceneHUD's ortho pass)
 * and hands it to Fan + Ground before drawing them. Also owns PickAt():
 * unproject the click at the near and far planes into a world-space ray,
 * test it against every pickable part's cached model matrix, keep the
 * nearest hit.
 */

#include "Scene3D.h"
#include "Fan.h"
#include "Ground.h"
#include "Transform.h"
#include "Platform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cfloat>
#include <cmath>
#include <algorithm>
#include <utility>

namespace {

// Ray-vs-oriented-box test: transform the world-space ray into the part's
// own local [-0.5, 0.5]^3 cube space (inverse of its model matrix) and do a
// standard slab test there. This is exact for every part regardless of its
// rotation or non-uniform scale -- including the thin, elongated blades,
// which the quiz spec calls out as needing more than a single point/sphere
// test (a segment/box test, which this is, handles them correctly).
bool RayIntersectsPart(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                        const glm::mat4& modelMatrix, float& outWorldT)
{
    glm::mat4 invModel = glm::inverse(modelMatrix);
    glm::vec3 lo = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 ld = glm::vec3(invModel * glm::vec4(rayDir, 0.0f));

    float tMin = -FLT_MAX;
    float tMax =  FLT_MAX;

    for (int axis = 0; axis < 3; ++axis) {
        if (std::fabs(ld[axis]) < 1e-6f) {
            if (lo[axis] < -0.5f || lo[axis] > 0.5f) return false;
            continue;
        }
        float t1 = (-0.5f - lo[axis]) / ld[axis];
        float t2 = ( 0.5f - lo[axis]) / ld[axis];
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    if (tMax < 0.0f) return false; // box is entirely behind the ray origin

    float tLocal = (tMin >= 0.0f) ? tMin : tMax;
    glm::vec3 hitLocal = lo + ld * tLocal;
    glm::vec3 hitWorld = glm::vec3(modelMatrix * glm::vec4(hitLocal, 1.0f));
    outWorldT = glm::length(hitWorld - rayOrigin);
    return true;
}

} // namespace

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* mgr)
    : fan(new Fan(mgr)), ground(new Ground(mgr))
{
}
#else
Scene3D::Scene3D()
    : fan(new Fan()), ground(new Ground())
{
}
#endif

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
    fan->InitModel();
    ground->InitModel();
}

glm::mat4 Scene3D::BuildViewMatrix() const
{
    // Same camera the fan used to bake straight into its own model matrix
    // in Quiz 2 (Translate(0,0.8,-8) -> Rotate(20,Y)), just now living in
    // Scene3D and driven by an adjustable distance instead of a constant.
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.8f, -cameraDistance));
    view = glm::rotate(view, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return view;
}

glm::mat4 Scene3D::BuildProjectionMatrix() const
{
    return glm::perspective(glm::radians(60.0f), aspect_, 0.01f, 1000.0f);
}

void Scene3D::Resize(int w, int h)
{
    if (h <= 0) h = 1;
    viewportW_ = w;
    viewportH_ = h;
    aspect_ = static_cast<float>(w) / static_cast<float>(h);
    glViewport(0, 0, w, h);

    // Forwarded per the spec, even though both are now driven entirely by
    // the matrices Scene3D passes in every Render() -- keeps the Model
    // interface symmetric if either one ever needs its own resize logic.
    fan->Resize(w, h);
    ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float d)
{
    cameraDistance = glm::clamp(d, 3.0f, 20.0f);
}

void Scene3D::SetSpeed(float speed)
{
    fan->SetSpeed(glm::clamp(speed, 0.0f, 20.0f));
}

void Scene3D::Render()
{
    glm::mat4 view = BuildViewMatrix();
    glm::mat4 proj = BuildProjectionMatrix();

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // The fan's cylinder/sphere/blade meshes are hand-generated (Fan.cpp);
    // culling is disabled here rather than trusting every hand-derived
    // winding order to be perfect -- for solid opaque shapes viewed from
    // outside, culling is a pure optimization, so this costs nothing
    // visually while removing an entire class of "silently invisible
    // geometry" bugs.
    glDisable(GL_CULL_FACE);

    ground->SetViewProjection(view, proj);
    ground->SetLightPos(lightPos_);
    ground->Render();

    fan->SetViewProjection(view, proj);
    fan->SetLightPos(lightPos_);
    fan->Render();

    glEnable(GL_CULL_FACE);
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    glm::mat4 view = BuildViewMatrix();
    glm::mat4 proj = BuildProjectionMatrix();
    int viewport[4] = { 0, 0, viewportW_, viewportH_ };

    // Mouse/touch Y is top-left origin; TransformUnproject expects the GL
    // bottom-left convention, so flip it here -- SceneHUD never needs this
    // because its ortho projection already matches top-left pixels directly.
    float flippedY = static_cast<float>(viewportH_) - screenY;

    Transform unprojector; // stateless helper -- only used for its math, no GL state touched
    float nearX, nearY, nearZ, farX, farY, farZ;
    unprojector.TransformUnproject(screenX, flippedY, 0.0f, &view, &proj, viewport, &nearX, &nearY, &nearZ);
    unprojector.TransformUnproject(screenX, flippedY, 1.0f, &view, &proj, viewport, &farX, &farY, &farZ);

    glm::vec3 rayOrigin(nearX, nearY, nearZ);
    glm::vec3 rayDir = glm::normalize(glm::vec3(farX - nearX, farY - nearY, farZ - nearZ));

    int bestPart = -1;
    float bestT = FLT_MAX;
    for (int i = 0; i < fan->GetPartCount(); ++i) {
        float t;
        if (RayIntersectsPart(rayOrigin, rayDir, fan->GetPartMatrix(i), t)) {
            if (t < bestT) {
                bestT = t;
                bestPart = i;
            }
        }
    }

    if (bestPart == -1) {
        // Empty space clears every current highlight, not just one.
        fan->ClearSelection();
        LOGI("Pick: empty space -- selection cleared");
        return false;
    }

    // Multi-select: each part's highlight toggles independently, so picking
    // a second part adds to the selection instead of stealing it from the first.
    fan->ToggleSelected(bestPart);
    LOGI("Pick: %s %s", fan->GetPartName(bestPart), fan->IsSelected(bestPart) ? "selected" : "deselected");
    return true;
}
