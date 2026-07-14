/**
 * Renderer.cpp
 *
 * Only file allowed to know Fan exists (inside createModels).
 * Everything else in the project talks to Model*.
 */

#include "Renderer.h"
#include "Fan.h"

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
    models.push_back(new Fan(assetMgr));
#else
    models.push_back(new Fan());
#endif
}

void Renderer::initializeModels()
{
    for (Model* m : models) {
        m->InitModel();
    }
}

void Renderer::clearModels()
{
    for (Model* m : models) {
        delete m;
    }
    models.clear();
}

bool Renderer::initializeRenderer()
{
    createModels();
    initializeModels();
    return true;
}

void Renderer::resize(int w, int h)
{
    for (Model* m : models) {
        m->Resize(w, h);
    }
}

void Renderer::render()
{
    for (Model* m : models) {
        m->Render();
    }
}

void Renderer::touchDown(float x, float y)
{
    for (Model* m : models) {
        m->TouchEventDown(x, y);
    }
}

void Ren