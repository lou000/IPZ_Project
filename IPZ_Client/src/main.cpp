#include <chrono>
#include <gl.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "asset_manager.h"
#include "shader.h"
#include "renderer.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



static void error_callback(int error, const char* description)
{
    WARN("GLFW ERROR: %d\n%s", error, description);
}

static void glErrorCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar* message, const void* userParam)
{
    UNUSED(length);
    UNUSED(userParam);

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

int main(void)
{

    GLFWwindow* window;

    ppk::assert::implementation::setAssertHandler(assertHandler);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(800, 800, "Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(glErrorCallback, 0);

    //This below is for RGB formats, if we stop supporting we can remove this.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    auto texture = std::make_shared<Texture>("../assets/img/test.png");
    AssetManager::addAsset(texture);
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));

    // NOTE: OpenGL error checks have been omitted for brevity

    Renderer::init();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            char title[50];
            double ms = 1000.0/double(nbFrames);
            sprintf_s(title, "Test - FPS: %.1f (%.2fms)", 1/ms*1000, ms);
            glfwSetWindowTitle(window, title);
            nbFrames = 0;
            lastTime = currentTime;
        }

        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();

        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        Renderer::begin();

        float w = 0.09f;
        float y = w/2;
        float x = w/2;
        vec4 colorStart = {0.7f,0.3f,0.7f,1.0f};
        vec4 colorEnd = {0.3f,0.2f,0.7f,1};
        while(x-w/2<4)
        {
            while(y-w/2<4)
            {
                float mixA = (float)(8-(y+x))/8;
                Renderer::DrawQuad({x-2,y-2,0}, {w, w}, nullptr, 1, mix(colorStart, colorEnd, mixA));
                y+=w+0.01f;
            }
            y = w/2;
            x+=w+0.01f;
        }
        Renderer::DrawQuad({0,0,0}, {2, 2}, texture);

        Renderer::end();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
