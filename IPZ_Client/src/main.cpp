#include <chrono>
#include <gl.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "asset_manager.h"
#include "shader.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

static const struct
{
    float x, y;
    float r, g, b;
} vertices[4] =
    {
        { -1.0f, -1.0f, 1.f, 0.f, 0.f },
        { -1.0f,  1.0f, 0.f, 1.f, 0.f },
        {  1.0f,  1.0f, 0.f, 0.f, 1.f },
        {  1.0f, -1.0f, 1.f, 1.f, 0.f }
};

static const GLubyte
    indices[] = {0,1,2,
                 0,2,3};


static void error_callback(int error, const char* description)
{
    ASSERT_WARNING(0, "GLFW ERROR: %d\n%s", error, description);
}

static void glErrorCallback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar* message, const void* userParam)
{
    UNUSED(length);
    UNUSED(userParam);

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
    UNUSED(_type); //clang woulnt shut up about whis

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        ASSERT_WARNING(0, "[OpenGL] %d: %s    SEVERITY: %s    FROM: %s\n%s", id, _type, "HIGH", _source, message);
        break;

    case GL_DEBUG_SEVERITY_MEDIUM:
        ASSERT_WARNING(0, "[OpenGL] %d: %s    SEVERITY: %s    FROM: %s\n%s", id, _type, "MEDIUM", _source, message);
        break;

    case GL_DEBUG_SEVERITY_LOW:
        ASSERT_DEBUG(0, "[OpenGL] %d: %s    SEVERITY: %s    FROM: %s\n%s", id, _type, "LOW", _source, message);
        break;

    case GL_DEBUG_SEVERITY_NOTIFICATION:
        ASSERT_DEBUG(0, "[OpenGL] %d: %s    SEVERITY: %s    FROM: %s\n%s", id, _type, "NOTIFY", _source, message);
        break;

    default:
        ASSERT_WARNING(0, "[OpenGL] %d: %s    SEVERITY: %s    FROM: %s\n%s", id, _type, "UNKNOWN", _source, message);
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
    GLuint vertex_buffer;
    GLint mvp_location, vpos_location, vcol_location;

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

    AssetManager::addAsset(std::make_shared<Texture>("../assets/img/test.png"));
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));

    // NOTE: OpenGL error checks have been omitted for brevity


    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    auto program = AssetManager::getShader("test");

    mvp_location = glGetUniformLocation(program->id(), "MVP");
    vpos_location = glGetAttribLocation(program->id(), "vPos");
    vcol_location = glGetAttribLocation(program->id(), "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));


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
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        m = mat4x4(1.0f);
        m = glm::rotate(m, (float) glfwGetTime(), vec3(0, 0, 1));
        p = glm::ortho(-2.0f,2.0f,-2.0f,2.0f,0.0f,100.0f);
        mvp = p * m;

        program->bind();
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE,(const GLfloat*) glm::value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
