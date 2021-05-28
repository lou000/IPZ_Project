#include "testga.h"
#include <map>
#define CITIES_COUNT 30
#define POPS_PER_LOOP 100
#define MAX_TIME 100


template<typename T>
GA<T>::~GA()
{
    free(m_population);
    free(m_fitnessScores);
}

template<typename T>
GA<T>::GA(uint popCount, uint geneCount, float mutateChance, float crossChance)
{
    m_popCount  = popCount;
    m_geneCount = geneCount;

    m_population       = (T*) malloc(popCount*geneCount*sizeof(T));
    m_fitnessScores    = (float*) malloc(popCount*sizeof(float));
    m_scoresNormalized = (float*) malloc(popCount*sizeof(float));

    m_mutateChance = mutateChance;
    m_crossChance  = crossChance;
}

template<typename T>
void GA<T>::initPopulation(std::function<void (T *, uint)> initFunc)
{
    for(uint p=0; p<m_popCount; p++)
        initFunc(m_population+(p*m_geneCount), m_geneCount);
}

template<typename T>
void GA<T>::select(std::function<uint (float*, uint)> selectFunc)
{
    T* selected = (T*) alloca(m_popCount*m_geneCount*sizeof(T));
    for(uint p=0; p<m_popCount; p++)
    {
        uint index = selectFunc(m_scoresNormalized, m_popCount);
        memcpy(selected+(p*m_geneCount), m_population+(index*m_geneCount), m_geneCount*sizeof(T));
    }
    memcpy(m_population, selected, m_popCount*m_geneCount*sizeof(T));
}

template<typename T>
void GA<T>::cross(std::function<void (T*, T*, uint)> crossFunc)
{
    for(uint p=0; p<m_popCount-1; p+=2)
    {
        float rand = glm::linearRand(0.f, 1.f);
        if(rand>=m_crossChance)
            crossFunc(m_population+(p*m_geneCount), m_population+((p+1)*m_geneCount), m_geneCount);
    }
}

template<typename T>
void GA<T>::mutate(std::function<void (T *, uint)> mutateFunc)
{
    for(uint p=0; p<m_popCount; p++)
    {
        float rand = glm::linearRand(0.f, 1.f);
        if(rand>=m_mutateChance)
            mutateFunc(m_population+(p*m_geneCount), m_geneCount);
    }
}

template<typename T>
void GA<T>::evaluate(std::function<float (T *, uint)> fitnessFunc)
{
    float sum = 0;
    for(uint p=0; p<m_popCount; p++)
    {
        float score = fitnessFunc(m_population+(p*m_geneCount), m_geneCount);
        m_fitnessScores[p] = score;
        sum += score;
    }
    for(uint p=0; p<m_popCount; p++)
    {
        m_scoresNormalized[p] = m_fitnessScores[p]/sum;
    }
    m_avgScore = sum/m_popCount;
}

template<typename T>
std::pair<T*, float> GA<T>::best()
{

    float best = -std::numeric_limits<float>::infinity();
    uint bestIndex = -1;
    for(uint p=0; p<m_popCount; p++)
    {
        if(m_fitnessScores[p]>best)
        {
            best = m_fitnessScores[p];
            bestIndex = p;
        }
    }
    return {m_population+(bestIndex*m_geneCount), best};
}

TestGA::TestGA()
{
    ga = new GA<uint>(200, CITIES_COUNT, 0.1f, 0.5f);
    cities = (vec2*) malloc(CITIES_COUNT*sizeof(vec2));
    indexes = (uint*) malloc(CITIES_COUNT*sizeof(uint));

    vec2 range = {0.f, 10.f};

    for(uint i=0; i<CITIES_COUNT; i++)
    {
        cities[i] = {glm::linearRand(range.x, range.y), glm::linearRand(range.x, range.y)};
        indexes[i] = i;
    }

    allTimeBest.setRange(range, range);
    allTimeBest.addPoints(cities, CITIES_COUNT, {1,0,0,1});
    allTimeBest.setPointScale(3.f);

    currentBest.setRange(range, range);
    currentBest.addPoints(cities, CITIES_COUNT, {1,0,0,1});
    currentBest.setPointScale(3.f);

    vec2 rangeX = {0, MAX_TIME};
    vec2 rangeY = {0, 1.f/CITIES_COUNT};

    avgPopScore.setRange(rangeX, rangeY);
    bestPerGen .setRange(rangeX, rangeY);
    bestOverall.setRange(rangeX, rangeY);

    fitFunc = [=](uint* genes, uint geneCount)
    {
        float distance = 0;
        for(uint i=0; i<geneCount-1; i++)
            distance += glm::distance(cities[genes[i]], cities[genes[i+1]]);
        return 1/distance;
    };
}

uint selectRulette(float* scores, uint popCount)
{
    float rand = glm::linearRand(0.f, 1.f);
    uint index = 0;
    while(rand>0)
    {
        ASSERT(index<popCount);
        rand -= scores[index];
        index++;
    }
    index--;
    return index;
}

void PMX(uint* x1, uint* x2, uint geneCount)
{
    uint cross1 = glm::linearRand((uint)1, geneCount-3);
    uint cross2 = glm::linearRand(cross1,  geneCount-2);

    int* desc1 = (int*)alloca(geneCount*sizeof(int));
    int* desc2 = (int*)alloca(geneCount*sizeof(int));
    memset(desc1, -1, geneCount*sizeof(int));
    memset(desc2, -1, geneCount*sizeof(int));

    std::map<uint, uint> map1;
    std::map<uint, uint> map2;
    for(uint i = cross1; i<cross2+1; i++)
    {
        desc1[i] = x2[i];
        desc2[i] = x1[i];
        map1.insert({x2[i], x1[i]});
        map2.insert({x1[i], x2[i]});
    }

    for(uint i=0; i<geneCount; i++)
    {
        if(desc1[i] != -1)
            continue;
        bool conflict = false;
        int g = (int)x1[i];
        for(uint j=0; j<geneCount; j++)
            if(desc1[j] == g)
                conflict = true;

        while(conflict)
        {
            conflict = false;
            g = map1[g];
            for(uint j=0; j<geneCount; j++)
                if(desc1[j] == g)
                    conflict = true;
        }
        desc1[i] = g;
    }

    for(uint i=0; i<geneCount; i++)
    {
        if(desc2[i] != -1)
            continue;
        bool conflict = false;
        int g = (int)x2[i];
        for(uint j=0; j<geneCount; j++)
            if(desc2[j] == g)
                conflict = true;

        while(conflict)
        {
            conflict = false;
            g = map2[g];
            for(uint j=0; j<geneCount; j++)
                if(desc2[j] == g)
                    conflict = true;
        }
        desc2[i] = g;
    }
    memcpy(x1, desc1, geneCount*sizeof(uint));
    memcpy(x2, desc2, geneCount*sizeof(uint));
}

void inverseMut(uint* genes, uint geneCount)
{
    uint cross1 = glm::linearRand((uint)1, geneCount-3);
    uint cross2 = glm::linearRand(cross1,  geneCount-2);

    while(cross1<cross2)
    {
        uint temp = genes[cross1];
        genes[cross1] = genes[cross2];
        genes[cross2] = temp;
        cross1++;
        cross2--;
    }
}

void TestGA::onStart()
{
    auto initFunc = [=](uint* genes, uint geneCount)
    {
        arrayShuffle(indexes, geneCount);
        memcpy(genes, indexes, geneCount*sizeof(uint));
    };
    ga->initPopulation(initFunc);
    ga->evaluate(fitFunc);
}
void TestGA::onUpdate(float dt)
{
    timePassed+=dt;

    auto best = ga->best();
    if(timePassed<MAX_TIME)
    {
        for(uint i=0; i<POPS_PER_LOOP; i++)
        {
            ga->select(selectRulette);
            ga->cross(PMX);
            ga->mutate(inverseMut);
            ga->evaluate(fitFunc);

            best = ga->best();

            if(best.second>bestScore)
            {
                LOG("bestScore: %f", bestScore);
                bestScore = best.second;
                memcpy(indexes, best.first, CITIES_COUNT*sizeof(uint));
            }
        }
        if(timePassed>prevLog+0.1f)
        {
            prevLog = timePassed;
            avgPopScore.addLine(prevAvg,         {timePassed, ga->avg()},   {0.969, 0.647, 0.627, 1});
            bestPerGen .addLine(prevBestGen,     {timePassed, best.second}, {0.835, 0.902, 0.733, 1});
            bestOverall.addLine(prevBestOverall, {timePassed, bestScore},   {0.443, 0.761, 0.776, 1});

            prevAvg         = {timePassed, ga->avg()};
            prevBestGen     = {timePassed, best.second};
            prevBestOverall = {timePassed, bestScore};
        }
    }


    vec2 winSize = App::getWindowSize();
    for(uint i=0; i<CITIES_COUNT-1; i++)
    {
        allTimeBest.addLine(cities[indexes[i]], cities[indexes[i+1]]);
        currentBest.addLine(cities[best.first[i]], cities[best.first[i+1]]);
    }

    auto width = winSize.x/2.f;
    auto height1 = winSize.y/2.f;
    auto height2 = winSize.y/3.f;
    allTimeBest.draw({0    , 0        }, winSize/2.f);
    currentBest.draw({0    , height1  }, winSize/2.f);
    avgPopScore.draw({width, 0        }, {width, height2});
    bestPerGen .draw({width, height2  }, {width, height2});
    bestOverall.draw({width, height2*2}, {width, height2});

    allTimeBest.clearLines();
    currentBest.clearLines();

}
