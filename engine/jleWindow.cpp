// Copyright (c) 2022. Johan Lind

#include "jleWindow.h"
#include "initWindow.h"
#include "jlePath.h"
#include "jlePathDefines.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

#include <GLFW/glfw3.h>

#include <iostream>

#include "stb_image.h"

jleWindow *jleWindow::activeWindow{nullptr};

void jleWindow::error_callback(int error, const char *description) {
    std::cerr << "GLFW ERROR: " << description << '\n';
}

void jleWindow::key_callback(
    GLFWwindow *window, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS) {
        sPressedKeys[key] = true;
    }

    if (action == GLFW_RELEASE) {
        sReleasedKeys[key] = true;
    }
}

void jleWindow::scroll_callback(GLFWwindow *window,
                                double xoffset,
                                double yoffset) {
    activeWindow->currentScrollX = static_cast<float>(xoffset);
    activeWindow->currentScrollY = static_cast<float>(yoffset);
}

void jleWindow::framebuffer_size_callback(GLFWwindow *window,
                                          int width,
                                          int height) {
    activeWindow->internalRenderingAPI->viewportDimensions(
        0,
        0,
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(height));

    activeWindow->windowSettings.width = static_cast<unsigned int>(width);
    activeWindow->windowSettings.height = static_cast<unsigned int>(height);

    // Call all subscribed callbacks
    activeWindow->executeResizeCallbacks(width, height);
}

#ifdef __EMSCRIPTEN__
extern "C" {
EMSCRIPTEN_KEEPALIVE
int resize_canvas_js(int width, int height) {

    glViewport(0, 0, width, height);

    printf("Change window size: %d, %d", width, height);

    const auto &window = jleWindow::activeWindow;

    jleWindow::framebuffer_size_callback(nullptr, width, height);

    return 1;
}
}
#endif

float jleWindow::time() { return static_cast<float>(glfwGetTime()); }

float jleWindow::scrollX() { return activeWindow->currentScrollX; }

float jleWindow::scrollY() { return activeWindow->currentScrollY; }

jleWindow::~jleWindow() {
    glfwDestroyWindow(nativeWindow);
    glfwTerminate();
}

void jleWindow::settings(WindowSettings &windowSettings) {
    this->windowSettings = windowSettings;
}

void jleWindow::displayCursor(bool enable) {
    if (enable) {
        glfwSetInputMode(nativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(nativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    cursorVisible = enable;
}

bool jleWindow::isCursorDisplayed() { return cursorVisible; }

unsigned int jleWindow::GetWindowHeight() { return windowSettings.height; }

unsigned int jleWindow::GetWindowWidth() { return windowSettings.width; }

void jleWindow::initWindow(std::shared_ptr<jleRendering> internalRenderingAPI) {
    if (!internalRenderingAPI) {
        std::cerr << "Rendering API is null!\n";
        exit(1);
    }

    this->internalRenderingAPI = internalRenderingAPI;

    activeWindow = this;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        std::cerr << "GLFW ERROR: COULD NOT INITIALIZE";
        exit(1);
    }

    if (windowSettings.isRezisable) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }
    else {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    nativeWindow = reinterpret_cast<GLFWwindow *>(
        ::initWindow(windowSettings.width,
                     windowSettings.height,
                     windowSettings.WindowTitle.c_str()));

    glfwSetKeyCallback(nativeWindow, key_callback);
    glfwSetScrollCallback(nativeWindow, scroll_callback);
    glfwSetFramebufferSizeCallback(nativeWindow, framebuffer_size_callback);

    glfwSetWindowSizeLimits(nativeWindow,
                            windowSettings.widthMin,
                            windowSettings.heightMin,
                            GLFW_DONT_CARE,
                            GLFW_DONT_CARE);

    displayCursor(windowSettings.shouldDisplayCursor);

#ifdef __EMSCRIPTEN__
    glfwSwapInterval(1);
#else
    if (windowSettings.isCappedFps) {
        glfwSwapInterval(1);
    }
    else {
        glfwSwapInterval(0);
    }
#endif

    int w, h;
    glfwGetFramebufferSize(nativeWindow, &w, &h);
    internalRenderingAPI->viewportDimensions(
        0, 0, static_cast<unsigned int>(w), static_cast<unsigned int>(h));

    if (!windowSettings.iconPath.empty()) {
        GLFWimage images[1];
        images[0].pixels = stbi_load(
            jleAbsolutePath(windowSettings.iconPath).absolutePathStr().c_str(),
            &images[0].width,
            &images[0].height,
            nullptr,
            4);
#ifndef __linux__
        glfwSetWindowIcon(nativeWindow, 1, images);
#endif
        stbi_image_free(images[0].pixels);
    }
}

void jleWindow::updateWindow() {
    activeWindow->currentScrollX = 0.f;
    activeWindow->currentScrollY = 0.f;

    std::memset(sPressedKeys, 0, sizeof(sPressedKeys));
    std::memset(sReleasedKeys, 0, sizeof(sPressedKeys));
    glfwPollEvents();
    glfwSwapBuffers(nativeWindow);
}

bool jleWindow::windowShouldClose() {
    return glfwWindowShouldClose(nativeWindow);
}

bool jleWindow::key(char key) { return glfwGetKey(nativeWindow, key); }

bool jleWindow::keyPressed(char key) { return sPressedKeys[key]; }

bool jleWindow::keyReleased(char key) { return sReleasedKeys[key]; }

std::pair<int, int> jleWindow::cursor() {
    double x, y;
    glfwGetCursorPos(nativeWindow, &x, &y);
    return std::pair<int, int>(static_cast<int>(x), static_cast<int>(y));
}

unsigned int jleWindow::addWindowResizeCallback(
    std::function<void(unsigned int, unsigned int)> callback) {
    unsigned int i = 0;

    // Find first available callback id
    for (auto it = windowResizedCallbacks.cbegin(),
              end = windowResizedCallbacks.cend();
         it != end && i == it->first;
         ++it, ++i) {
    }

    windowResizedCallbacks.insert(std::make_pair(
        i, std::bind(callback, std::placeholders::_1, std::placeholders::_2)));

    return i;
}

void jleWindow::removeWindowResizeCallback(unsigned int callback_id) {
    windowResizedCallbacks.erase(callback_id);
}

void jleWindow::executeResizeCallbacks(int w, int h) {
    for (const auto &callback : activeWindow->windowResizedCallbacks) {
        callback.second(w, h);
    }
}

bool jleWindow::mouseClick(int button) {
    return glfwGetMouseButton(nativeWindow, button);
}
