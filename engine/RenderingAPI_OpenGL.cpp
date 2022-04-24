#include "RenderingAPI_OpenGL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

#include "iQuadRenderingInternal.h"
#include "iTextRenderingInternal.h"
#include "plog/Log.h"

namespace jle
{
	void RenderingAPI_OpenGL::SetViewportDimensions(int x, int y, unsigned int width, unsigned int height)
	{
		glViewport(x, y, static_cast<int>(width), static_cast<int>(height));
	}

	void RenderingAPI_OpenGL::Render(iFramebuffer& framebufferOut)
	{
		((iQuadRenderingInternal*)quads.get())->Render(framebufferOut);
		((iTextRenderingInternal*)texts.get())->Render(framebufferOut);
	}

	void RenderingAPI_OpenGL::Setup(const iRenderingFactory& renderFactory)
	{
        LOG_VERBOSE << "Creating text rendering";
        this->texts = renderFactory.CreateTextRendering();
        LOG_VERBOSE << "Creating quad rendering";
		this->quads = renderFactory.CreateQuadRendering();

	}
}


