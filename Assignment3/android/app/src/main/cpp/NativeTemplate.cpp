/**
 * NativeTemplate.cpp
 *
 * JNI bridge for the Android activity. All calls run on the GLSurfaceView GL
 * thread; scene logic remains in the shared Renderer and Fan classes.
 */

#define LOG_TAG "Fan3D"

#include <jni.h>
#include <android/asset_manager_jni.h>

#include "Renderer.h"

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_fan3d_MainActivity_nativeInit(
        JNIEnv* env, jobject, jobject javaAssetManager)
{
    AAssetManager* manager = AAssetManager_fromJava(env, javaAssetManager);
    Renderer::Instance().setAssetManager(manager);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeResize(
        JNIEnv*, jobject, jint width, jint height)
{
    Renderer::Instance().resize(static_cast<int>(width), static_cast<int>(height));
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeRender(JNIEnv*, jobject)
{
    Renderer::Instance().render();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchDown(
        JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().touchEventDown(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchMove(
        JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().touchEventMove(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchRelease(
        JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().touchEventRelease(x, y);
}
