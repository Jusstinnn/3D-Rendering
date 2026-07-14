/**
 * main.cpp
 *
 * Entry point for Desktop (Windows/GLFW) and WebGL (Emscripten/GLFW) builds.
 * Android uses NativeTemplate.cpp + JNI instead of this file.
 *
 * Desktop/web mouse -> touch mapping: left mouse button press/release maps
 * to TouchEventDown/TouchEventRelease; cursor movement while the button is
 * held maps to TouchEventMove. This is the same three-call pipeline Android
 * drives through MainActivity.java's queueEvent + JNI.
 */

#include "Platform.h"
#include "Renderer.h"

// ==========================================================================
// WebGL / Emscripten (GLFW backend, built with -DUSE_GLFW -s USE_GLFW=3)
// ==========================================================================
#ifdef PLATFORM_EMSCRIPTEN

static GLFWwindow* g_window   = nullptr;
static bool        g_dragging = false;

static void fbsize_cb(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void mouseButton_cb(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    if (action == GLFW_PRESS) {
        g_dragging = true;
        Renderer::Instance().touchDown(static_cast<float>(x), static_cast<float>(y));
    } else if (action == GLFW_RELEASE) {
        g_dragging = false;
        Renderer::Instance().touchRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

static void cursorPos_cb(GLFWwindow* /*win*/, double x, double y)
{
    if (g_dragging) {
        Renderer::Instance().touchMove(static_cast<float>(x), static_cast<float>(y));
    }
}

static void main_loop()
{
    if (glfwWindowShouldClose(g_window)) {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return;
    }
    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(g_window);
}

int main()
{
    LOGI("3D Fan - WebGL / Emscripten (GLFW)");

    if (!glfwInit()) { LOGE("glfwInit failed"); return -1; }

    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(800, 600, "3D Fan", nullptr, nullptr);
    if (!g_window) { LOGE("glfwCreateWindow failed"); glfwTerminate(); return -1; }

    glfwMakeContextCurrent(g_window);
    glfwSetFramebufferSizeCallback(g_window, fbsize_cb);
    glfwSetMouseButtonCallback(g_window, mouseButton_cb);
    glfwSetCursorPosCallback(g_window, cursorPos_cb);

    Renderer::Instance().initializeRenderer();
    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    Renderer::Instance().resize(w, h);

    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

// ==========================================================================
// Desktop - Windows / GLFW
// ==========================================================================
#elif defined(PLATFORM_WINDOWS)

#include <iostream>

static bool g_dragging = false;

static void framebufferSizeCB(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void mouseButtonCB(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    if (action == GLFW_PRESS) {
        g_dragging = true;
        Renderer::Instance().touchDown(static_cast<float>(x), static_cast<float>(y));
    } else if (action == GLFW_RELEASE) {
        g_dragging = false;
        Renderer::Instance().touchRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

static void cursorPosCB(GLFWwindow* /*win*/, double x, double y)
{
    if (g_dragging) {
        Renderer::Instance().touchMove(static_cast<float>(x), static_cast<float>(y));
    }
}

int main()
{
    std::cout << "3D Fan - Desktop (GLFW)\n";
    std::cout << "Click to toggle the fan. Click-drag fast to spin it up. Press ESC to quit.\n";

    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return -1; }

    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Fan", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "glewInit failed\n"; return -1; }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCB);
    glfwSetMouseButtonCallback(window, mouseButtonCB);
    glfwSetCursorPosCallback(window, cursorPosCB);

    Renderer::Instance().initializeRenderer();
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    Renderer::Instance().resize(w, h);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        Renderer::Instance().render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#endif // PLATFORM_WINDOWS
