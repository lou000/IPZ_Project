#include "utilities.h"
#include "application.h"

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
        OPENGL_THROW("%s: %d   SEVERITY: MEDIUM   FROM: %s\n%s", _type, id, _source, message);
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

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void App::x_init(uint width, uint height)
{

    if (!glfwInit())
    {
        ASSERT_FATAL(0, "glfwInit failed!");
        exit(EXIT_FAILURE);
    }

    ppk::assert::implementation::setAssertHandler(assertHandler);
    glfwSetErrorCallback(error_callback);

    m_windowHeight = height;
    m_windowWidth  = width;
    m_window = glfwCreateWindow(width, height, "Test", NULL, NULL);
    if (!m_window)
    {
        ASSERT_FATAL(0, "Couldnt create a window!");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(m_window, key_callback);
    glfwMakeContextCurrent(m_window);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(m_window, &App::mouseScrollCallback);
    gladLoadGL();
    glEnable(GL_DEBUG_OUTPUT);

    //This below is for RGB formats, if we stop supporting we can remove this.
    glDebugMessageCallback(glErrorCallback, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    Renderer::setViewPort({0,0}, {width, height});
    Renderer::setClearColor({0.302, 0.345, 0.388, 1});

    glfwSwapInterval(1);
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
    float dt = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
    return dt;
}

vec2 App::x_getMousePosChange()
{
    dvec2 currentMousePos;
    glfwGetCursorPos(m_window, &currentMousePos.x, &currentMousePos.y);
    vec2 mouseChange = (vec2)currentMousePos - m_prevMousePos;
    m_prevMousePos = (vec2)currentMousePos;

    //the problem is that first when we call this function mouseChange is huge
    //this is a very ugly hack, later we will use glfwSetCursorPosCallback();
//    if(length(mouseChange)>500)
//        return {0,0};
    return mouseChange;
}

double App::x_getMouseScrollChange()
{
    double offset = mouseScrollYOffset;
    mouseScrollYOffset = 0;
    return offset;
}

void App::x_mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    UNUSED(window);
    UNUSED(xoffset);
    LOG("bigOffset: %f", yoffset);
    mouseScrollYOffset += yoffset;
}

bool App::x_shouldClose()
{
    return glfwWindowShouldClose(m_window);
}
