#include "testrosenblat.h"
#include "gtc/random.hpp"
#include <mutex>


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
    points = (Point*)malloc(nPoints * sizeof(Point));
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
        points[i] = p;
    }


    // random centers
    centers = (vec2*)malloc(nCenters * sizeof(vec2));
    for(int i=0; i<nCenters; i++)
        centers[i] = {glm::linearRand(-1.f, 1.f), glm::linearRand(-1.f, 1.f)};

    // features
    int count = 0;
    features = (float*) malloc(nPoints * nCenters * sizeof(float));
    for(int i=0; i<nPoints; i++)
        for(int j=0; j<nCenters; j++)
        {
            features[count] = calcFeature(points[i].pos, centers[j], fi);
            count++;
        }

    // weights
    weights = (float*) malloc((nCenters+1) * sizeof(float));
    for(int i=0; i<nCenters+1; i++)
        weights[i] = glm::linearRand(0.f, 1.f);

    // mesh grid
    meshGrid = (Point*)malloc(meshSize*sizeof(Point));
    for(int i=0; i<meshX; i++)
        for(int j=0; j<meshX; j++)
        {
            Point p;
            p.pos.y = -1+i*meshStep;
            p.pos.x = -1+j*meshStep;
            p.val = 0;
            meshGrid[i*meshX+j] = p;
        }
}

float TestRosenblat::calcFeature(vec2 point, vec2 center, float fi)
{
    float sx = point.x - center.x;
    float sy = point.y - center.y;
    float qs = sx*sx + sy*sy;
    return exp(-(qs/(2*fi*fi)));
}

void TestRosenblat::countour()
{

    // calculate values of grid

    auto meshGridPtr = meshGrid;
    auto nThreads = tPool.get_thread_count();

    int count = meshSize/nThreads;

    for(uint i=0; i<nThreads; i++)
    {
        if(i == nThreads-1)
            count = meshSize-count*(nThreads-1);
        tPool.submit([=](){
            for(int i=0; i<count; i++)
            {
                float sum = 0;

                for(int j=0; j<nCenters; j++)
                {
                    sum += calcFeature((meshGridPtr+i)->pos, centers[j], fi)*weights[j];
                }
                sum += weights[nCenters];
                (meshGridPtr+i)->val = sum;
            }
            return;
        });
        meshGridPtr+=count;
    }
    tPool.wait_for_tasks();


    // very simple marching sqares solution to draw the outline
    vec2 offset = {mapToRange({0,2}, {0, winSize.x-margin*2}, meshStep),
                   mapToRange({0,2}, {0, winSize.y-margin*2}, meshStep)};
    vec2 halfOffset = offset/2.f;
    float lwidth = 2;
    vec4 color = {0.629, 1.000, 0.688, 1};
    for(int i=0; i<meshX-1; i++)
    {
        for(int j=0; j<meshX-1; j++)
        {
            int vIndex = 0;
            int val0 = meshGrid[(i+1)*meshX+j].val  >0;
            int val1 = meshGrid[(i+1)*meshX+j+1].val>0;
            int val2 = meshGrid[i*meshX+j+1].val    >0;
            int val3 = meshGrid[i*meshX+j].val      >0;

            vec2 p0 = {left+offset.x*j+halfOffset.x, top+offset.y*(i+1)};
            vec2 p1 = {left+offset.x*(j+1),          top+offset.y*i+halfOffset.y};
            vec2 p2 = {left+offset.x*j+halfOffset.x, top+offset.y*i};
            vec2 p3 = {left+offset.x*j,              top+offset.y*i+halfOffset.y};

            vIndex |= val0;
            vIndex |= val1<<1;
            vIndex |= val2<<2;
            vIndex |= val3<<3;

            switch(vIndex) {
            case 0:
                break;
            case 1:
                BatchRenderer::drawLine(p0, p3, lwidth, color);break;
            case 2:
                BatchRenderer::drawLine(p0, p1, lwidth, color);break;
            case 3:
                BatchRenderer::drawLine(p1, p3, lwidth, color);break;
            case 4:
                BatchRenderer::drawLine(p1, p2, lwidth, color);break;
            case 5:
                BatchRenderer::drawLine(p0, p1, lwidth, color);break;
                BatchRenderer::drawLine(p2, p3, lwidth, color);break;
            case 6:
                BatchRenderer::drawLine(p0, p2, lwidth, color);break;
            case 7:
                BatchRenderer::drawLine(p2, p3, lwidth, color);break;
            case 8:
                BatchRenderer::drawLine(p2, p3, lwidth, color);break;
            case 9:
                BatchRenderer::drawLine(p0, p2, lwidth, color);break;
            case 10:
                BatchRenderer::drawLine(p1, p2, lwidth, color);break;
                BatchRenderer::drawLine(p0, p3, lwidth, color);break;
            case 11:
                BatchRenderer::drawLine(p1, p2, lwidth, color);break;
            case 12:
                BatchRenderer::drawLine(p1, p3, lwidth, color);break;
            case 13:
                BatchRenderer::drawLine(p0, p1, lwidth, color);break;
            case 14:
                BatchRenderer::drawLine(p0, p3, lwidth, color);break;
            case 15:
                break;
            }
        }
    }
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
            if(out != points[i].val)
            {
                // this below is extremely dumb
                memcpy(badResults+brCount*(nCenters+1), features+i*nCenters, nCenters*sizeof(float));
                badResults[brCount*(nCenters+1)+nCenters] = points[i].val;
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
    tPool.sleep_duration = 0;
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
    for(int i=0; i<nPoints; i++)
    {
        auto p = points[i];
        vec2 pos = {mapToRange({-1,1}, {left, right}, p.pos.x),
                    mapToRange({-1,1}, {top, bottom}, p.pos.y)};
        vec4 color = points[i].val == -1 ? vec4(1.000, 0.502, 0.529, 1) : vec4(0.529, 0.682, 1.000, 1);
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
    tPool.sleep_duration = 1000;
}
