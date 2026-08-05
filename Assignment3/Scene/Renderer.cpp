/**
 * Renderer.cpp
 *
 * Only file allowed to know Scene3D/SceneHUD exist (inside createModels).
 * Implements the exact frame order the quiz spec requires (Part 1):
 *
 *   Clear -> Render Scene3D (depth ON) -> disable depth -> Render SceneHUD
 *   (ortho, on top of everything) -> re-enable depth -> [caller swaps buffers]
 *
 * and the touch-routing rule from Part 5/8: SceneHUD gets first look at
 * every press: if it consumes the press (a button was hit), Scene3D's 3D
 * picking never runs for that press.
 */

#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"
#include "Platform.h"

Renderer& Renderer::Instance()
{
    static Renderer instance;
    return instance;
}

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D  = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D  = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
}

void Renderer::clearModels()
{
    delete scene3D;
    scene3D = nullptr;
    delete sceneHUD;
    sceneHUD = nullptr;
}

bool Renderer::initializeRenderer()
{
    createModels();
    scene3D->InitModel();
    sceneHUD->InitModel();
    return true;
}

void Renderer::resize(int w, int h)
{
    scene3D->Resize(w, h);
    sceneHUD->Resize(w, h);
}

void Renderer::render()
{
    // SceneHUD is the source of truth for speed/zoom -- Scene3D owns what
    // those values *do*, SceneHUD just reports what the user pressed.
    scene3D->SetSpeed(sceneHUD->CurrentSpeed());
    scene3D->SetCameraDistance(sceneHUD->CurrentZoom());

    glEnable(GL_DEPTH_TEST);
    scene3D->Render(); // clears the framebuffer itself, depth-tested

    glDisable(GL_DEPTH_TEST); // a HUD button must never be hidden behind "closer" 3D geometry
    // SceneHUD's top-left-origin ortho projection flips Y relative to
    // Scene3D's perspective camera, which reverses the HUD quads' winding
    // order (CCW becomes CW in window space). Backface culling is still
    // enabled from Transform::TransformInit(), so without this the HUD
    // silently draws nothing -- flat 2D UI has no "back face" anyway.
    glDisable(GL_CULL_FACE);
    sceneHUD->Render();
    glEnable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST); // restore state for next frame's Scene3D pass
}

void Renderer::shutdown()
{
    clearModels();
}

void Renderer::touchEventDown(float x, float y)
{
    // HUD gets first refusal. Only a miss falls through to 3D picking.
    bool consumedByHUD = sceneHUD->TouchDown(x, y);
    if (!consumedByHUD) {
        scene3D->PickAt(x, y);
    }
}

void Renderer::touchEventMove(float /*x*/, float /*y*/)
{
    // Buttons in this quiz fire once on press, not while dragging; nothing
    // to route here. (Left as an explicit no-op, not deleted, so a future
    // press-and-hold bonus has an obvious place to live.)
}

void Renderer::touchEventRelease(float /*x*/, float /*y*/)
{
    sceneHUD->TouchRelease();
}
