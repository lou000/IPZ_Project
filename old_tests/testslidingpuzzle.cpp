#include "testslidingpuzzle.h"

TestSlidingPuzzle::TestSlidingPuzzle()
{
    for(int i=1; i<=35; i++)
        AssetManager::addAsset(std::make_shared<Texture>("../assets/img/numero"+std::to_string(i)+".png"));

    BatchRenderer::setShader(AssetManager::getShader("batch"));

    n = 5;
    size = n*n;
    center = (float)n/2-0.1f;

    puzzle = new SlidePuzzle(n, std::chrono::steady_clock::now().time_since_epoch().count(), SlidePuzzle::Manhattan);
    searcher = new informative_searcher(*puzzle, &SlidePuzzle::compare);
    solutionPath = searcher->get_solution_path(0);
    iter = solutionPath.end();
    iter--;

}

void TestSlidingPuzzle::onStart()
{
    auto camera = activeCamera();
    camera->setFov(50.f);
    camera->setPosition({center, 8, 4});
    camera->setFocusPoint({center,0,center});
}

void TestSlidingPuzzle::onUpdate(float dt)
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

    BatchRenderer::begin(GraphicsContext::getCamera()->getViewProjectionMatrix());
    BatchRenderer::drawQuad({center,-0.001,center}, {n, n}, vec4(0.459, 0.349, 0.298, 1));
    for(int i=0; i<size; i++)
    {
        float x = 0.4f+i%n;
        float z = 0.4f+i/n;
        vec3 pos = {x, 0, z};
        std::shared_ptr<Texture> tileText = nullptr;
        if(grid[i]!= 0)
            tileText = std::dynamic_pointer_cast<Texture>(AssetManager::getAsset("../assets/img/numero"+std::to_string(grid[i])+".png"));

        // Here be animation
        if(i == movingFromIndx)
        {
            pos += movDir * animProgress;
            BatchRenderer::drawQuad(pos, {0.8f, 0.8f}, tileText);
            animProgress+=animationSpeed*dt;
        }
        else if( grid[i]!= 0)
            BatchRenderer::drawQuad(pos, {0.8f, 0.8f}, tileText);
    }
    BatchRenderer::end();
}
