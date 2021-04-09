#pragma once
#include <chrono>
#include "asset_manager.h"
#include "shader.h"
#include "renderer.h"


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
    static float getTimeStep(){return getInstance().x_getTimeStep();}
    static vec2 getMousePosChange(){return getInstance().x_getMousePosChange();}
    static double getMouseScrollChange(){return getInstance().x_getMouseScrollChange();}
    static bool getKeyOnce(int key, int mods = 0, int action = GLFW_PRESS){return getInstance().x_getKeyOnce(key, mods, action);}

private:
    GLFWwindow* m_window;
    uint m_windowHeight = 0;
    uint m_windowWidth = 0;
    float m_lastFrameTime = 0;
    vec2 m_prevMousePos = {0,0};
    double mouseScrollYOffset = 0;
    // this is probably very wrong way of doing this its only temporary
    std::vector<size_t> keyHashBuffer;

    void x_init(uint width, uint height);
    void x_setVsync(uint interval);
    void x_setWindowTitle(const std::string& title);
    void x_submitFrame();
    void x_quit();
    double x_getTime();
    uvec2 x_getWindowSize();
    bool x_shouldClose();
    GLFWwindow* x_getWindowHandle();
    float x_getTimeStep();
    vec2 x_getMousePosChange();
    double x_getMouseScrollChange();
    bool x_getKeyOnce(int key, int mods, int action);


    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {return getInstance().x_keyCallback(window, key, scancode, action, mods);}
    void x_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {return getInstance().x_mouseScrollCallback(window, xoffset, yoffset);}
    void x_mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

