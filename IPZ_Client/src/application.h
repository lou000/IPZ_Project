#pragma once
#include <chrono>
#include <gl.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "asset_manager.h"
#include "shader.h"
#include "renderer.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


class App
{
    App() = default;
    static App& getInstance(){
        static App instance;
        return instance;
    }
public:
    App(App const&)            = delete;
    void operator=(App const&) = delete;

    static void init(uint width, uint height){getInstance().x_init(width, height);}
    static void setVsync(uint interval){getInstance().x_setVsync(interval);}
    static void setWindowTitle(const std::string& title){getInstance().x_setWindowTitle(title);}
    static void submitFrame(){getInstance().x_submitFrame();}
    static void quit(){getInstance().x_quit();}
    static double getTime(){return getInstance().x_getTime();}
    static uvec2 getWindowSize(){return getInstance().x_getWindowSize();}
    static bool shouldClose(){return getInstance().x_shouldClose();}
    static GLFWwindow* getWindowHandle(){return getInstance().x_getWindowHandle();}

private:
    GLFWwindow* m_window;
    uint m_windowHeight = 0;
    uint m_windowWidth = 0;

    void x_init(uint width, uint height);
    void x_setVsync(uint interval);
    void x_setWindowTitle(const std::string& title);
    void x_submitFrame();
    void x_quit();
    double x_getTime();
    uvec2 x_getWindowSize();
    bool x_shouldClose();
    GLFWwindow* x_getWindowHandle();
};

