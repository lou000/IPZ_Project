#include "testrosenblat.h"
#include "gtc/random.hpp"


TestRosenblat::TestRosenblat()
{
    std::vector<std::filesystem::path> shaderSrcs = {
        "../assets/shaders/default_batch.fs",
        "../assets/shaders/default_batch.vs"
    };
    AssetManager::addShader(std::make_shared<Shader>("batch", shaderSrcs));
    BatchRenderer::setShader(AssetManager::getShader("batch"));

    rangeX = {0, 2*glm::pi<float>()};
    rangeY = {-1, 1};


    // random points
    for(int i=0; i<nPoints; i++)
    {
        Point p;
        p.pos.x = glm::linearRand(rangeX.x, rangeX.y);
        p.pos.y = glm::linearRand(rangeY.x, rangeY.y);
        if(abs(sin(p.pos.x)) > abs(p.pos.y))
            p.val = -1;
        else
            p.val =  1;
        p.pos.x = mapToRange(rangeX, {-1,1}, p.pos.x);
        p.pos.y = mapToRange(rangeY, {-1,1}, p.pos.y);
        points.push_back(p);
    }


    // random centers
    for(int i=0; i<nCenters; i++)
    {
        vec2 c = {glm::linearRand(-1.f, 1.f), glm::linearRand(-1.f, 1.f)};
        centers.push_back(c);
    }

    // features
    int count = 0;
    features = (float*) malloc(nPoints * nCenters * sizeof(float));
    for(auto p : points)
        for(auto c : centers)
        {
            features[count] = calcFeature(p.pos, c);
            count++;
        }

    // weights
    weights = (float*) malloc((nCenters+1) * sizeof(float));
    for(int i=0; i<nCenters+1; i++)
        weights[i] = glm::linearRand(0.f, 1.f);

    // mesh grid
    int x = (int)2/meshStep;
    for(int i=0; i<x; i++)
        for(int j=0; j<x; j++)
        {
            Point p;
            p.pos.x = -1+i*meshStep;
            p.pos.y = -1+j*meshStep;
            p.val = 0;
            meshGrid.push_back(p);
        }
}

float TestRosenblat::calcFeature(vec2 point, vec2 center)
{
    float sx = point.x - center.x;
    float sy = point.y - center.y;
    float qs = sx*sx + sy*sy;
    return exp(-(qs/(2*fi*fi)));
}

void TestRosenblat::countour()
{

    // check all points and if there is a sign diff on bottom or right draw countour there

    meshGrid[0].val = checkPoint(meshGrid[0]);
    for(size_t i=1; i<meshGrid.size()-1; i++)
    {

        int width = (int)2/meshStep;
        auto& cell   = meshGrid[i];
        auto prevCell = meshGrid[i-1];
        vec2 pos = {mapToRange({-1,1}, {left, right}, cell.pos.x),
                                mapToRange({-1,1}, {bottom, top}, cell.pos.y)};
        cell.val = checkPoint(cell);
        if((i-1)%width != 0)
        {
            if(cell.val != prevCell.val)
                BatchRenderer::drawCircle(pos, 4, 10, {0.629, 1.000, 0.688, 1});
        }
    }

}

float TestRosenblat::checkPoint(const Point& p)
{
    float sum = 0;
    for(int i=0; i<nCenters; i++)
    {
        sum += calcFeature(p.pos, centers[i])*weights[i];
    }
    sum += weights[nCenters];
    return sum>0 ? 1 : -1;
}

int TestRosenblat::train(uint times) // returns number of incorrect results
{
    int brCount = 0;
    float* badResults = (float*)alloca(nPoints*(nCenters+1)*sizeof(float)); //idk how much so yolo
    for(uint i=0; i<times; i++)
    {
        brCount = 0;
        for(int i=0; i<nPoints; i++)
        {

            float sum = 0;
            // sum all feature*weight
            for(int j=0; j<nCenters; j++)
            {
                sum += features[i*nCenters+j]*weights[j];
            }
            // add last weight
            sum += weights[nCenters];

            float out = sum>0 ? 1.0f : -1.0f;
            if(out != points.at(i).val)
            {
                // this below is extremely dumb
                memcpy(badResults+brCount*(nCenters+1), features+i*nCenters, nCenters*sizeof(float));
                badResults[brCount*(nCenters+1)+nCenters] = points.at(i).val;
                brCount++;
            }
        }


        auto randI = glm::linearRand(0, brCount-1);
        auto v  = badResults[randI*(nCenters+1)+nCenters];
        for(int i=0; i<nCenters; i++)
        {
            auto br = badResults[randI*(nCenters+1)+i];
            weights[i] += learnFactor*br*v;
        }
        weights[nCenters] += learnFactor*v;
    }
    return brCount;
}

void TestRosenblat::onUpdate(float dt)
{
    winSize = App::getWindowSize();
    margin = winSize.y /10;
    gridSpacing = winSize.y / 10;
    bottom = winSize.y - margin;
    top    = margin;
    right  = winSize.x - margin;
    left   = margin;
    vec2 topLeft     = {left,top};
    vec2 topRight    = {right, top};
    vec2 bottomRight = {right, bottom};
    vec2 bottomLeft  = {left, bottom};

    BatchRenderer::begin();

    // Draw background
    BatchRenderer::drawQuad({0,0}, winSize, {0.051, 0.067, 0.090, 1});

    // Draw Axes
    BatchRenderer::drawLine(topLeft, bottomLeft, 5.f, {0.788, 0.820, 0.851,1});
    BatchRenderer::drawLine({left-2.5, bottom}, bottomRight, 5.f, {0.788, 0.820, 0.851,1});

    // Draw grid
    for(float i=left+gridSpacing; i<right; i+=gridSpacing)
        BatchRenderer::drawLine({i, bottom}, {i, top}, 2.f, {0.788, 0.820, 0.851, 0.5});
    for(float i=bottom-gridSpacing; i>top; i-=gridSpacing)
        BatchRenderer::drawLine({left, i}, {right, i}, 2.f, {0.788, 0.820, 0.851, 0.5});

    // Draw points
    for(auto p : points)
    {
        vec2 pos = {mapToRange({-1,1}, {left, right}, p.pos.x),
                    mapToRange({-1,1}, {bottom, top}, p.pos.y)};
        vec4 color = p.val == -1 ? vec4(1.000, 0.502, 0.529, 1) : vec4(0.529, 0.682, 1.000, 1);
        BatchRenderer::drawCircle(pos, 3, 10, color);
    }

//    // Draw centers
//    for(auto c : centers)
//    {
//        vec2 pos = {mapToRange({-1,1}, {left, right}, c.x),
//                    mapToRange({-1,1}, {bottom, top}, c.y)};
//        BatchRenderer::drawCircle(pos, 8, 10, {1, 1.000, 0.288, 1});
//    }

    // train
    if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
        trainPerFrame -= 1;
    if(App::getKeyOnce(GLFW_KEY_KP_ADD))
        trainPerFrame += 1;

    if(k<maxK)
    {
        int bCount = train(trainPerFrame);
        if(bCount == 0)
            k = maxK;
        else
        {
            k+=trainPerFrame;
        }
    }

    // countour slice of hyperspace
    countour();

    BatchRenderer::end();
}
