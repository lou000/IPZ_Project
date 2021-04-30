#include "graphicscontext.h"
#include "../Core/application.h"

void GraphicsContext::x_init()
{
    auto winSize = App::getWindowSize();
    m_currentCamera = std::make_shared<Camera>(90, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    BatchRenderer::init();
    MeshRenderer::init();
}

void GraphicsContext::setClearColor(vec4 color)
{
    glClearColor(color.r, color.g, color.b, 1);
}

void GraphicsContext::clear(GLbitfield buffers)
{
    glClear(buffers);
}

void GraphicsContext::x_setCamera(std::shared_ptr<Camera> camera)
{
    m_currentCamera = camera;
}

void GraphicsContext::x_resizeViewPort(int width, int height)
{
    glViewport(0,0, width, height);
    m_currentCamera->setAspectRatio((float)width/(float)height);
}
