#include "test1.h"
Test1::Test1()
{
    texture = std::make_shared<Texture>("../assets/img/test.png");
    texture2 = std::make_shared<Texture>("../assets/img/bomb.png");
    AssetManager::addAsset(texture);
    AssetManager::addAsset(texture2);
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));
    auto winSize = App::getWindowSize();
    auto camera = std::make_shared<Camera>(40.f, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    ImRender::setCamera(camera);
    camera->setPosition({0,5.4,0});

    for(int i=0; i<1000; i++)
        randomPos.push_back({rndDouble(0, 6), rndDouble(0, 6)});

    auto renderable = std::make_shared<TexturedQuad>("BasicQuad", AssetManager::getShader("test"), MAX_VERTEX_BUFFER_SIZE);
    ImRender::addRenderable(renderable);
}

void Test1::onUpdate(float dt)
{

    ImRender::begin("BasicQuad");
    float w = 0.09f;
    float z = w/2;
    float x = w/2;
    vec4 colorStart = {0.969, 0.588, 0.498,1.0f};
    vec4 colorEnd = {0.255, 0.263, 0.478,1};
    ImRender::DrawQuad({0,0.001,0}, {4, 4}, vec4(0.090, 0.059, 0.286, 1));
    while(x-w/2<4)
    {
        while(z-w/2<4)
        {
            float mixA = clamp((float)(8-(z+x))/8+0.25f, 0.f, 1.0f);
            ImRender::DrawQuad({x-2, 0.002,z-2}, {w, w}, mix(colorStart, colorEnd, mixA));
            z+=w+0.01f;
        }
        z = w/2;
        x+=w+0.01f;
    }

    float y = 0.0001;
    for(auto& pos : randomPos)
    {
        pos.x += (float)rndInt(-2, 1)/200;;
        pos.y += (float)rndInt(-2, 1)/200;
        pos = mod(pos, {6,6});
        ImRender::DrawQuad({pos.x-3, y, pos.y-3}, {0.3f, 0.3f}, texture2);
        y+=0.0001;
    }
    ImRender::DrawQuad({0,0.1,0}, {2, 2}, texture);
    ImRender::end();
}
