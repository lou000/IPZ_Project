#include "utilities.h"
#include "application.h"
#include "../Renderer/graphicscontext.h"

static void error_callback(int error, const char* description)
{
    UNUSED(error); //clang cant see us using it clearly in a macro below...
    UNUSED(description);
    WARN("GLFW ERROR: %d\n%s", error, description);
}

static void glErrorCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar* message, const void* userParam)
{
    UNUSED(length);
    UNUSED(userParam);
    UNUSED(message);

    if(id == 131185)
        return;

    const char* _source;
    const char* _type;

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;

    case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;

    case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;

    case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;

    default:
        _source = "UNKNOWN";
        break;
    }
    UNUSED(_source); //clang wouldnt shut up about this

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;

    case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;

    case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;

    case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;

    case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;

    default:
        _type = "UNKNOWN";
        break;
    }
    UNUSED(_type); //clang woulnt shut up about this

    //TODO: see if we can pass file and line from the opengl error callsite

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        OPENGL_THROW("%s: %d   SEVERITY: HIGH   FROM: %s\n%s", _type, id, _source, message);
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        OPENGL_LOG("%s: %d   SEVERITY: MEDIUM   FROM: %s\n%s", _type, id, _source, message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        OPENGL_LOG("%s: %d   SEVERITY: LOW   FROM: %s\n%s", _type, id, _source, message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        //Just log here dont assert
        OPENGL_LOG("%s: %d   SEVERITY: NOTIFICATION   FROM: %s\n%s", _type, id, _source, message);
        break;

    default:
        OPENGL_THROW("%s: %d   SEVERITY: UNKNOWN   FROM: %s\n%s", _type, id, _source, message);
        break;
    }
}

void App::x_init(uint width, uint height)
{

    keyBuffer.reserve(KEY_BUFFER_SIZE);

    glfwInitHint(GLFW_WIN32_MESSAGES_IN_FIBER, GLFW_TRUE);
    if (!glfwInit())
    {
        ASSERT_FATAL(0, "glfwInit failed!");
        exit(EXIT_FAILURE);
    }

    ppk::assert::implementation::setAssertHandler(assertHandler);
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_SAMPLES, 16);
    m_windowHeight = height;
    m_windowWidth  = width;
    m_window = glfwCreateWindow(width, height, "Test", NULL, NULL);
    if (!m_window)
    {
        ASSERT_FATAL(0, "Couldnt create a window!");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(m_window);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(m_window, &App::keyCallback);
    glfwSetScrollCallback(m_window, &App::mouseScrollCallback);
    glfwSetMouseButtonCallback(m_window, &App::mouseButtonCallback);
    glfwSetFramebufferSizeCallback(m_window, &App::frameBufferSizeCallback);

    gladLoadGL();
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glErrorCallback, 0);

    GraphicsContext::init();

    glfwSwapInterval(1);
}

bool App::x_getKey(int key, KeyActionFlags actionFlags, int mods)
{
    UNUSED(mods); // this will not work for mods, maybe we can make it work?
    int action = glfwGetKey(m_window, key);
    action = action == 0 ? 1 : action<<1;
    if(actionFlags & action)
        return true;
    return false;
}

bool App::x_getKeyOnce(int key, KeyActionFlags actionFlags, int mods)
{

    uint16 hash = 0;
    hash |= key << 9;
    hash |= mods << 3;

    for(auto h : keyBuffer)
        if((h>>3)<<3==hash) // check if the hash matches without 3 bottom bits
        {
            auto bottomBits = h & 0x7;  //compare bottom bits to our flags
            if(actionFlags & bottomBits)
                return true;
        }

    return false;
}

bool App::x_getMouseButton(int button, KeyActionFlags actionFlags, int mods)
{
    uint16 hash = 0;
    hash |= button << 9;
    hash |= mods << 3;
    ASSERT(!(actionFlags & REPEAT), "App: There is no REPEAT action for mouse.");
    for(auto h : mouseButtonBuffer)
        if((h>>3)<<3==hash) // check if the hash matches without 3 bottom bits
        {
            auto bottomBits = h & 0x7;  //compare bottom bits to our flags
            if(actionFlags & bottomBits)
                return true;
        }

    return false;
}

bool App::x_getMouseButtonHeld(int button, int mods)
{
    uint16 hash = 0;
    hash |= button << 6;
    hash |= mods;
    for(auto h : mouseHeldBuffer)
        if(h == hash)
            return true;
    return false;
}

void App::x_disableCursor(bool disable)
{
    if(disable)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    else
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void App::x_keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    UNUSED(window);
    UNUSED(scancode);
    //just jam them in one var
    uint16 hash = 0;
    hash |= key << 9;
    hash |= mods << 3;
    // convert glfw action to our flag
    hash |= action == 0 ? 1 : action<<1;

    keyBuffer.push_back(hash);
}
void App::x_mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    UNUSED(window);
    //just jam them in one var
    uint16 hash = 0;
    hash |= button << 9;
    hash |= mods << 3;
    // convert glfw action to our flag
    int _action = action == 0 ? 1 : action<<1;
    hash |= _action;

    mouseButtonBuffer.push_back(hash);

    //If its pressed add button if its released remove it
    hash = hash>>3;
    if(_action & PRESS)
        mouseHeldBuffer.push_back(hash);
    else
    {
        for(size_t i=0; i<mouseHeldBuffer.size(); i++)
            if(mouseHeldBuffer[i] == hash)
                mouseHeldBuffer.erase(mouseHeldBuffer.begin()+i);
    }
}

void App::x_frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
    //this might be called before we initialize the Renderer
    UNUSED(window);
    m_windowWidth  = width;
    m_windowHeight = height;
    GraphicsContext::resizeViewPort(width, height);
}

void App::x_setVsync(uint interval)
{
    glfwSwapInterval(interval);
}

void App::x_setWindowTitle(const std::string &title)
{
    glfwSetWindowTitle(m_window, title.c_str());
}

void App::x_submitFrame()
{
    keyBuffer.clear();
    mouseButtonBuffer.clear();
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void App::x_quit()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

double App::x_getTime()
{
    return glfwGetTime();
}

uvec2 App::x_getWindowSize()
{
    return {m_windowWidth, m_windowHeight};
}

GLFWwindow* App::x_getWindowHandle()
{
    return m_window;
}

float App::x_getTimeStep()
{
    float currentFrame = (float)glfwGetTime();
    float dt = currentFrame - m_lastFrameTime;
    m_lastFrameTime = currentFrame;
    return dt;
}

vec2 App::x_getMousePos()
{
    dvec2 currentMousePos;
    glfwGetCursorPos(m_window, &currentMousePos.x, &currentMousePos.y);
    return (vec2) currentMousePos;
}

vec2 App::x_getMousePosChange()
{
    dvec2 currentMousePos;
    glfwGetCursorPos(m_window, &currentMousePos.x, &currentMousePos.y);
    vec2 mouseChange = (vec2)currentMousePos - m_prevMousePos;
    m_prevMousePos = (vec2)currentMousePos;

    return mouseChange;
}

float App::x_getMouseScrollChange()
{
    float offset = mouseScrollYOffset;
    mouseScrollYOffset = 0;
    return offset;
}

void App::x_mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    UNUSED(window);
    UNUSED(xoffset);
    mouseScrollYOffset += (float)yoffset;
}

bool App::x_shouldClose()
{
    return glfwWindowShouldClose(m_window);
}
