#include "application.h"
#include "slidepuzzle.h"

int main(void)
{

    App::init(800, 800);
    App::setVsync(1);
    auto texture = std::make_shared<Texture>("../assets/img/test.png");
    auto texture2 = std::make_shared<Texture>("../assets/img/bomb.png");
    AssetManager::addAsset(texture);
    AssetManager::addAsset(texture2);
    for(int i=1; i<=15; i++)
        AssetManager::addAsset(std::make_shared<Texture>("../assets/img/numero"+std::to_string(i)+".png"));

    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/test_frag.glsl",
        "../assets/shaders/test_vert.glsl"
    };
    AssetManager::addShader(std::make_shared<Shader>("test", shaderSrcs));
    Renderer::init();

    // FPS counter should go to App
    float dtSum = 0;
    int frameCount = 0;

    std::vector<vec2> randomPos;
    for(int i=0; i<1000; i++)
        randomPos.push_back({rndDouble(0, 6), rndDouble(0, 6)});

    auto winSize = App::getWindowSize();
    auto camera = std::make_shared<Camera>(40.f, (float)winSize.x/(float)winSize.y, 0.1f, 1000.f);
    int n = 4;
    int size = n*n;
    float center = (float)n/2-0.1f;
    camera->setPosition({center, 8, 4});
    camera->setFocusPoint({center,0,center});
    camera->pointAt({center,0,center});
    Renderer::setCamera(camera);
    auto renderable = std::make_shared<TexturedQuad>("BasicQuad", AssetManager::getShader("test"), MAX_VERTEX_BUFFER_SIZE);
    Renderer::addRenderable(renderable);
    auto puzzle = SlidePuzzle(n, std::chrono::steady_clock::now().time_since_epoch().count(), SlidePuzzle::Manhattan);
    auto searcher = informative_searcher(puzzle, &SlidePuzzle::compare);
    auto solutionPath = searcher.get_solution_path(0);
    auto iter = solutionPath.end();
    iter--;
    float animationSpeed = 2.f;
    float animProgress = 1.0f;
    while (!App::shouldClose())
    {
        if(App::getKeyOnce(GLFW_KEY_SPACE))
            animationSpeed = animationSpeed > 0 ? 0 : 2.f;
        if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
            animationSpeed -= 1.f;
        if(App::getKeyOnce(GLFW_KEY_KP_ADD))
            animationSpeed += 1.f;
        if(App::getKeyOnce(GLFW_KEY_R))
        {
            iter = solutionPath.end();
            animProgress = 0;
            iter--;
        }

        float dt = App::getTimeStep();
        dtSum+=dt;
        frameCount++;
        if (frameCount == 20){
            char title[50];
            double ms = dtSum/double(frameCount) * 1000;
            sprintf_s(title, "Test - FPS: %.1f (%.2fms)", 1/ms*1000, ms);
            App::setWindowTitle(title);
            frameCount = 0;
            dtSum = 0;
        }

        camera->onUpdate(dt); //this will go in entity manager
        AssetManager::checkForChanges();
        AssetManager::tryReloadAssets();


        Renderer::begin("BasicQuad");
        if(animProgress>=1.0f && iter != solutionPath.begin())
        {
            iter--;
            animProgress = 0.0f;
        }
        auto grid = ((SlidePuzzle*)*iter)->getData();
        //this is peak c++ confusion
        auto nextGrid = iter == solutionPath.begin() ? ((SlidePuzzle*)*iter)->getData() : ((SlidePuzzle*)*std::prev(iter))->getData();

        //Find changed indexes
        int movingFromIndx = -1;
        int movingToIndx   = -1;
        for(int i=0; i<size; i++)
        {
            if(grid[i] != nextGrid[i])
            {
                if(grid[i]!=0)
                    movingFromIndx = i;
                else
                    movingToIndx = i;
            }
        }
        //Find movement vector
        vec3 movDir = {0,0,0};
        int rowFrom = movingFromIndx / n;
        int rowTo   = movingToIndx / n;
        if(rowFrom == rowTo)
        {
            if(movingFromIndx>movingToIndx)
                movDir.x = -1;
            else
                movDir.x = 1;
        }
        else
        {
            if(movingFromIndx>movingToIndx)
                movDir.z = -1;
            else
                movDir.z = 1;
        }

        Renderer::DrawQuad({center,0,center}, {n, n}, vec4(0.459, 0.349, 0.298, 1));
        for(int i=0; i<size; i++)
        {
            float x = 0.4f+i%n;
            float z = 0.4f+i/n;
            vec3 pos = {x, 0, z};
            auto tileText = std::dynamic_pointer_cast<Texture>(AssetManager::getAsset("../assets/img/numero"+std::to_string(grid[i])+".png"));

            // Here be animation
            if(i == movingFromIndx)
            {
                pos += movDir * animProgress;
                Renderer::DrawQuad(pos, {0.8f, 0.8f}, tileText);
                animProgress+=animationSpeed*dt;
            }
            else if( grid[i]!= 0)
                Renderer::DrawQuad(pos, {0.8f, 0.8f}, tileText);
        }

#if 0
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
#endif

        Renderer::end();
        App::submitFrame();
    }
}
