#pragma once

/**
 * Renderer.h
 *
 * Singleton controller -- the only thing main.cpp / NativeTemplate.cpp talk
 * to. Quiz 3 (Part 1): now owns a Scene3D and a SceneHUD directly instead
 * of a Fan, since the touch pipeline needs to route "did the HUD consume
 * this click" before falling through to 3D picking -- not just broadcast
 * every event to every Model unconditionally.
 */

#include "Model.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();
    void shutdown();

    // Forwarded from MainActivity.java (via queueEvent + JNI) on Android,
    // and from the GLFW mouse callbacks in main.cpp on Desktop/Web.
    void touchEventDown(float x, float y);
    void touchEventMove(float x, float y);
    void touchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D*  scene3D  = nullptr;
    SceneHUD* sceneHUD = nullptr;
};
