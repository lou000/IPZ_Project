#pragma once
#include "camera.h"

//TODO: here be things that are shared by both renderers
// initialize renderers from here, manage the camera
// add common render commands
// resize viewport from here
class GraphicsContext
{
public:
    GraphicsContext();
    static void setClearColor(vec4 color);
    static void clear();
    static void setCamera(std::shared_ptr<Camera> camera);
    static void getCamera(std::shared_ptr<Camera> camera);
    static void resizeViewPort(int width, int height);

private:
    std::shared_ptr<Camera> currentCamera;
};

