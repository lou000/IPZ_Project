#pragma once
#include "camera.h"

//TODO: here be things that are shared by both renderers
// initialize renderers from here, manage the camera
// add common render commands
// resize viewport from here
class GraphicsContext
{
    GraphicsContext() = default;
    static GraphicsContext& getInstance(){
        static GraphicsContext instance;
        return instance;
    }
public:
    GraphicsContext(GraphicsContext const&)   = delete;
    void operator=(GraphicsContext const&) = delete;

    static void setClearColor(vec4 color);
    static void clear(GLbitfield buffers);
    static void setCamera(std::shared_ptr<Camera> camera){getInstance().x_setCamera(camera);}
    static std::shared_ptr<Camera> getCamera(){return getInstance().x_getCamera();}
    static void resizeViewPort(int width, int height){getInstance().x_resizeViewPort(width, height);}

private:
    std::shared_ptr<Camera> m_currentCamera;

    void x_init();
    void x_setCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> x_getCamera(){return m_currentCamera;}
    void x_resizeViewPort(int width, int height);
};

