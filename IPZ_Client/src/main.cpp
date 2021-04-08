#include "application.h"

int main(void)
{

    App::init(800, 800);
    auto texture = std::make_shared<Texture>("../assets/img/test.png");
    auto texture2 = std::make_shared<Texture>("../assets/img/bomb.png");
    AssetManager::addAsset(texture);
    AssetManager::addAsset(texture2);
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));
    Renderer::init();

    // FPS counter should go to App
    double lastTime = App::getTime();
    int nbFrames = 0;

    std::vector<vec2> randomPos;
    for(int i=0; i<1000; i++)
        randomPos.push_back({rndDouble(0, 6), rndDouble(0, 6)});

    auto winSize = App::getWindowSize();
    auto camera = std::make_shared<Camera>(90.f, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);

    while (!App::shouldClose())
    {
        float dt = App::getTimeStep();
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

        camera->onUpdate(dt); //this will go in entity manager
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();


        Renderer::begin(camera);

        float w = 0.09f;
        float z = w/2;
        float x = w/2;
        vec4 colorStart = {0.969, 0.588, 0.498,1.0f};
        vec4 colorEnd = {0.255, 0.263, 0.478,1};
        Renderer::DrawQuad({0,0,0}, {4, 4}, vec4(0.090, 0.059, 0.286, 1));
        while(x-w/2<4)
        {
            while(z-w/2<4)
            {
                float mixA = clamp((float)(8-(z+x))/8+0.25f, 0.f, 1.0f);
                Renderer::DrawQuad({x-2, 0,z-2}, {w, w}, mix(colorStart, colorEnd, mixA));
                z+=w+0.01f;
            }
            z = w/2;
            x+=w+0.01f;
        }

        for(auto& pos : randomPos)
        {
            pos.x += (float)rndInt(-2, 1)/200;;
            pos.y += (float)rndInt(-2, 1)/200;
            pos = mod(pos, {6,6});
            Renderer::DrawQuad({pos.x-3, 0, pos.y-3}, {0.3f, 0.3f}, texture2);
        }
        Renderer::DrawQuad({0,0,0}, {2, 2}, texture);

        Renderer::DrawQuad({0,0,0}, {0.05f, 0.05f},{1,1,1,1});

        Renderer::end();
        App::submitFrame();
    }
}
