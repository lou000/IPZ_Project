#pragma once
#include "camera.h"

class GraphicsContext
{
    GraphicsContext() = default;
    static GraphicsContext& getInstance(){
        static GraphicsContext instance;
        return instance;
    }
public:
    GraphicsContext(GraphicsContext const&)= delete;
    void operator=(GraphicsContext const&) = delete;

    static void init(){getInstance().x_init();}
    static ivec2 getViewPortSize(){return getInstance().x_getViewPortSize();}
    static void resizeViewPort(int width, int height){getInstance().x_resizeViewPort(width, height);}

private:
    ivec2 m_viewPortSize;

    void x_init();
    ivec2 x_getViewPortSize(){return m_viewPortSize;}
    void x_resizeViewPort(int width, int height);
};

