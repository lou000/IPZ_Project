#include "graphicscontext.h"
#include "../Core/application.h"

void GraphicsContext::x_init()
{
    auto winSize = App::getWindowSize();
    m_viewPortSize = winSize;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glClearColor(0.302f, 0.345f, 0.388f, 1.f);

    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    BatchRenderer::init();
}

void GraphicsContext::x_resizeViewPort(int width, int height)
{
    if(width>0 && height>0)
    {
        m_viewPortSize = {width, height};
        glViewport(0,0, width, height);
    }
}
