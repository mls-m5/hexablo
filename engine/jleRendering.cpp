// Copyright (c) 2022. Johan Lind

#include "jleRendering.h"

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else

#include <glad/glad.h>

#endif

#include "jleProfiler.h"
#include "jleQuadRendering.h"
#include "jleTextRendering.h"
#include "plog/Log.h"

void jleRendering::SetViewportDimensions(int x,
                                         int y,
                                         unsigned int width,
                                         unsigned int height) {
    glViewport(x, y, static_cast<int>(width), static_cast<int>(height));
}

void jleRendering::Render(Framebuffer_OpenGL& framebufferOut,
                          jleCamera& camera) {
    JLE_SCOPE_PROFILE(jleRendering::Render)
    _quads->QueueRender(framebufferOut, camera);
    _texts->Render(framebufferOut, camera);
}

void jleRendering::Setup() {
    LOG_VERBOSE << "Creating text rendering";
    this->_texts = std::make_unique<jleTextRendering>();
    LOG_VERBOSE << "Creating quad rendering";
    this->_quads = std::make_unique<jleQuadRendering>();
}

void jleRendering::ClearBuffersForNextFrame() {
    _quads->ClearBuffersForNextFrame();
    _texts->ClearBuffersForNextFrame();
}

jleQuadRendering& jleRendering::quads() { return *_quads; }

jleTextRendering& jleRendering::texts() { return *_texts; }