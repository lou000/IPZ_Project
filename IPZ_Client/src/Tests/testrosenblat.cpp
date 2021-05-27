#include "testrosenblat.h"
#include "gtc/random.hpp"
#include "gtx/compatibility.hpp"
#include <mutex>


TestRosenblat::TestRosenblat()
{
    BatchRenderer::setShader(AssetManager::getShader("batch"));

    rangeX = {0, 2*glm::pi<float>()};
    rangeY = {-1, 1};


    // random points
    std::vector<vec2> pointsDrawIn;
    std::vector<vec2> pointsDrawOut;
    points = (Point*)malloc(nPoints * sizeof(Point));
    for(int i=0; i<nPoints; i++)
    {
        Point p;
        p.pos.x = glm::linearRand(rangeX.x, rangeX.y);
        p.pos.y = glm::linearRand(rangeY.x, rangeY.y);
        if(abs(sin(p.pos.x)) > abs(p.pos.y))
        {
            p.val = -1;
            pointsDrawIn.push_back(p.pos);
        }
        else
        {
            p.val =  1;
            pointsDrawOut.push_back(p.pos);
        }

        // normalize
        p.pos.x = mapToRange(rangeX, {-1,1}, p.pos.x);
        p.pos.y = mapToRange(rangeY, {-1,1}, p.pos.y);
        points[i] = p;
    }

    graph.addPoints(pointsDrawIn.data(),  (uint)pointsDrawIn.size(),  vec4(1.000, 0.502, 0.529, 1));
    graph.addPoints(pointsDrawOut.data(), (uint)pointsDrawOut.size(), vec4(0.529, 0.682, 1.000, 1));


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
    graph.setRange(rangeX, rangeY);
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
    auto len = length(rangeY);
    vec2 offset = {mapToRange({0,2}, rangeX, meshStep),
                   meshStep};

    vec4 color = {0.629, 1.000, 0.688, 1};
    for(int i=0; i<meshX-1; i++)
    {
        for(int j=0; j<meshX-1; j++)
        {
            int vIndex = 0;
            auto cell0 = meshGrid[(i+1)*meshX+j];
            auto cell1 = meshGrid[(i+1)*meshX+j+1];
            auto cell2 = meshGrid[i*meshX+j+1];
            auto cell3 = meshGrid[i*meshX+j];

            // positions of sqaure vertices
            vec2 p0 = {rangeX.x+offset.x*j,     rangeY.x+offset.y*(i+1)};
            vec2 p1 = {rangeX.x+offset.x*(j+1), rangeY.x+offset.y*(i+1)};
            vec2 p2 = {rangeX.x+offset.x*(j+1), rangeY.x+offset.y*i};
            vec2 p3 = {rangeX.x+offset.x*j,     rangeY.x+offset.y*i};

            // points between vertices where value should be 0
            vec2 q0 = lerp(p0, p1, abs(cell0.val)/(abs(cell0.val)+abs(cell1.val)));
            vec2 q1 = lerp(p1, p2, abs(cell1.val)/(abs(cell1.val)+abs(cell2.val)));
            vec2 q2 = lerp(p2, p3, abs(cell2.val)/(abs(cell2.val)+abs(cell3.val)));
            vec2 q3 = lerp(p3, p0, abs(cell3.val)/(abs(cell3.val)+abs(cell0.val)));

            vIndex |= (cell0.val>0);
            vIndex |= (cell1.val>0)<<1;
            vIndex |= (cell2.val>0)<<2;
            vIndex |= (cell3.val>0)<<3;

            switch(vIndex) {
            case 0:
                break;
            case 1:
                graph.addLine(q0, q3, color);break;
            case 2:
                graph.addLine(q0, q1, color);break;
            case 3:
                graph.addLine(q1, q3, color);break;
            case 4:
                graph.addLine(q1, q2, color);break;
            case 5:
                graph.addLine(q0, q1, color);break;
                graph.addLine(q2, q3, color);break;
            case 6:
                graph.addLine(q0, q2, color);break;
            case 7:
                graph.addLine(q2, q3, color);break;
            case 8:
                graph.addLine(q2, q3, color);break;
            case 9:
                graph.addLine(q0, q2, color);break;
            case 10:
                graph.addLine(q1, q2, color);break;
                graph.addLine(q0, q3, color);break;
            case 11:
                graph.addLine(q1, q2, color);break;
            case 12:
                graph.addLine(q1, q3, color);break;
            case 13:
                graph.addLine(q0, q1, color);break;
            case 14:
                graph.addLine(q0, q3, color);break;
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

void TestRosenblat::onStart()
{

}

void TestRosenblat::onUpdate(float dt)
{
    auto winSize = App::getWindowSize();

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

    graph.draw({0, 0}, winSize);
    graph.clearLines();
}
