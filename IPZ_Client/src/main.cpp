#include "application.h"

int main(void)
{


//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); ????
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    App::init(700, 700);
    auto texture = std::make_shared<Texture>("../assets/img/test.png");
    AssetManager::addAsset(texture);
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));
    Renderer::init();

    double lastTime = App::getTime();
    int nbFrames = 0;

    while (!App::shouldClose())
    {
        double currentTime = App::getTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            char title[50];
            double ms = 1000.0/double(nbFrames);
            sprintf_s(title, "Test - FPS: %.1f (%.2fms)", 1/ms*1000, ms);
            App::setWindowTitle(title);
            nbFrames = 0;
            lastTime = currentTime;
        }
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();


        Renderer::begin();

        float w = 0.09f;
        float y = w/2;
        float x = w/2;
        vec4 colorStart = {0.7f,0.3f,0.7f,1.0f};
        vec4 colorEnd = {0.3f,0.2f,0.7f,1};
        Renderer::DrawQuad({0,0,0}, {4, 4}, nullptr, 1, vec4(0.067, 0.078, 0.106, 1));
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
        App::submitFrame();
    }
}
