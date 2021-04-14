#pragma once
#include <chrono>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "asset_manager.h"
#include "shader.h"
#include "renderer.h"

#define KEY_BUFFER_SIZE 30 //this should be enough?

typedef uint8 KeyActionFlags;

class App
{
    App() = default;
    inline static App& getInstance(){
        static App instance;
        return instance;
    }
public:
    enum KeyAction{
        RELEASE = 1,
        PRESS   = 2,
        REPEAT  = 4
    };

    App(App const&)            = delete;
    void operator=(App const&) = delete;

    static void init(uint width, uint height){getInstance().x_init(width, height);}
    static void submitFrame()               {getInstance().x_submitFrame();}
    static void quit()                       {getInstance().x_quit();}

    static void setVsync(uint interval)                  {getInstance().x_setVsync(interval);}
    static void disableCursor(bool disable)             {getInstance().x_disableCursor(disable);}
    static void setWindowTitle(const std::string& title){getInstance().x_setWindowTitle(title);}

    static bool shouldClose()            {return getInstance().x_shouldClose();}
    static uvec2 getWindowSize()         {return getInstance().x_getWindowSize();}
    static GLFWwindow* getWindowHandle(){return getInstance().x_getWindowHandle();}
    static double getTime()              {return getInstance().x_getTime();}
    static float getTimeStep()           {return getInstance().x_getTimeStep();}

    static vec2 getMousePosChange()     {return getInstance().x_getMousePosChange();}
    static float getMouseScrollChange() {return getInstance().x_getMouseScrollChange();}
    static bool getKey(int key, KeyActionFlags actionFlags = PRESS | REPEAT, int mods = 0)
                {return getInstance().x_getKey(key, actionFlags, mods);}
    static bool getKeyOnce(int key, KeyActionFlags actionFlags = PRESS | REPEAT, int mods = 0)
                {return getInstance().x_getKeyOnce(key, actionFlags, mods);}
    static bool getMouseButton(int key, KeyActionFlags actionFlags = PRESS, int mods = 0)
                {return getInstance().x_getMouseButton(key, actionFlags, mods);}
    static bool getMouseButtonHeld(int key, int mods = 0)
                {return getInstance().x_getMouseButtonHeld(key, mods);}

private:
    GLFWwindow* m_window;
    uint m_windowHeight = 0;
    uint m_windowWidth = 0;
    float m_lastFrameTime = 0;
    vec2 m_prevMousePos = {0,0};
    float mouseScrollYOffset = 0;
    // this is probably very wrong way of doing this its only temporary
    std::vector<uint16> keyBuffer;
    std::vector<uint16> mouseButtonBuffer;
    std::vector<uint16> mouseHeldBuffer;

    void x_init(uint width, uint height);
    void x_submitFrame();
    void x_quit();

    void x_setVsync(uint interval);
    void x_disableCursor(bool disable);
    void x_setWindowTitle(const std::string& title);

    bool x_shouldClose();
    uvec2 x_getWindowSize();
    GLFWwindow* x_getWindowHandle();
    double x_getTime();
    float x_getTimeStep();

    vec2 x_getMousePosChange();
    float x_getMouseScrollChange();
    bool x_getKey(int key, KeyActionFlags actionFlags, int mods);
    bool x_getKeyOnce(int key, KeyActionFlags actionFlags, int mods);
    bool x_getMouseButton(int button, KeyActionFlags actionFlags, int mods);
    bool x_getMouseButtonHeld(int button, int mods);


    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {return getInstance().x_keyCallback(window, key, scancode, action, mods);}
    void x_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {return getInstance().x_mouseScrollCallback(window, xoffset, yoffset);}
    void x_mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
    {return getInstance().x_mouseButtonCallback(window, button, action, mods);}
    void x_mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
    {getInstance().x_frameBufferSizeCallback(window, width, height);}
    void x_frameBufferSizeCallback(GLFWwindow* window, int width, int height);
};

