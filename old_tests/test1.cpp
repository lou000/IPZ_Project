#include "test1.h"
Test1::Test1()
{
    texture = std::make_shared<Texture>("../assets/img/test.png");
    texture2 = std::make_shared<Texture>("../assets/img/bomb.png");
    AssetManager::addAsset(texture);
    AssetManager::addAsset(texture2);

    BatchRenderer::setShader(AssetManager::getShader("batch"));


    for(int i=0; i<1000; i++)
        randomPos.push_back({glm::linearRand<double>(0, 6), glm::linearRand<double>(0, 6)});

}

void Test1::onStart()
{
    auto camera = GraphicsContext::getCamera();
    camera->setFov(50.f);
    camera->setPosition({0,3.0f,0.1f});
    camera->setFocusPoint({0,0,0});
}

void Test1::onUpdate(float dt)
{

    BatchRenderer::begin(GraphicsContext::getCamera()->getViewProjectionMatrix());
    float w = 0.09f;
    float z = w/2;
    float x = w/2;
    vec4 colorStart = {0.969, 0.588, 0.498,1.0f};
    vec4 colorEnd = {0.255, 0.263, 0.478,1};
    BatchRenderer::drawQuad({0,0.001,0}, {4, 4}, vec4(0.090, 0.059, 0.286, 1));
    while(x-w/2<4)
    {
        while(z-w/2<4)
        {
            float mixA = clamp((float)(8-(z+x))/8+0.25f, 0.f, 1.0f);
            BatchRenderer::drawQuad({x-2, 0.002,z-2}, {w, w}, mix(colorStart, colorEnd, mixA));
            z+=w+0.01f;
        }
        z = w/2;
        x+=w+0.01f;
    }

    float y = 0.0001;
    for(auto& pos : randomPos)
    {
        pos.x += (float)glm::linearRand<int>(-2, 1)/200;;
        pos.y += (float)glm::linearRand<int>(-2, 1)/200;
        pos = mod(pos, {6,6});
        BatchRenderer::drawQuad({pos.x-3, y, pos.y-3}, {0.3f, 0.3f}, texture2);
        y+=0.0001;
    }
    BatchRenderer::drawQuad({0,0.1,0}, {2, 2}, texture);
    BatchRenderer::end();
}
