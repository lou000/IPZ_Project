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
    static void setClearColor(vec4 color);
    static void clear(GLbitfield buffers);
    static void setCamera(std::shared_ptr<Camera> camera){getInstance().x_setCamera(camera);}
    static std::shared_ptr<Camera> getCamera(){return getInstance().x_getCamera();}
    static ivec2 getViewPortSize(){return getInstance().x_getViewPortSize();}
    static void resizeViewPort(int width, int height){getInstance().x_resizeViewPort(width, height);}

private:
    std::shared_ptr<Camera> m_currentCamera;
    ivec2 m_viewPortSize;

    void x_init();
    void x_setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> x_getCamera(){return m_currentCamera;}
    ivec2 x_getViewPortSize(){return m_viewPortSize;}
    void x_resizeViewPort(int width, int height);
};

