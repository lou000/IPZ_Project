#include "testcontagionsim.h"
#define POP_COUNT 1000
#define ARENA_X 160.f
#define ARENA_Y 90.f
#define DAYS_PER_SEC 1.f
#define CONTACT_RADIUS 0.5f

TestContagionSim::TestContagionSim()
{

}

float resistanceRoulette(float age)
{
    if(age<15 || age>=70)
    {
        return glm::linearRand<float>(0.f, 3.f);
    }
    if(age>=15 && age<40)
    {
        return glm::linearRand<float>(3.f, 6.f);
    }
    if(age>=40 && age<70)
    {
        return glm::linearRand<float>(6.f, 10.f);
    }
    else
    {
        ASSERT(0);
        return 0;
    }
}

float maxAgeResistance(float age)
{
    if(age<15 || age>=70)
        return 3.f;
    if(age>=15 && age<40)
        return 6.f;
    if(age>=40 && age<70)
        return 10.f;
    else
    {
        ASSERT(0);
        return 0;
    }
}

void TestContagionSim::PopMember::changeState(MemberState state)
{
    this->state = state;
    switch (state) {
    case sick:
        this->stateCountdown = 7;
        break;
    case infected:
        this->stateCountdown = 2;
        break;
    case recovering:
        this->stateCountdown = 5;
        break;
    case healthy:
    case dead:
        this->stateCountdown = 0;
        break;
    }
}

void TestContagionSim::PopMember::evalState()
{
    if(this->stateCountdown<=0)
        switch (state)
        {
        case sick:
            this->changeState(recovering);
            break;
        case infected:
            this->changeState(sick);
            break;
        case recovering:
            this->changeState(healthy);
            break;
        case healthy:
        case dead:
            break;
        }
}



void TestContagionSim::simulate(float dt)
{
    for(auto& member : population)
    {
        // wall bounce
        if(member.pos.x<=0 || member.pos.x>=ARENA_X || member.pos.y>=ARENA_Y || member.pos.y <= 0)
            member.direction = glm::normalize(glm::circularRand(1.f));

        // collision
        // this is extremely slow and inefficiant
        for(auto m2 : population)
        {
            if(glm::distance(member.pos, m2.pos)<=CONTACT_RADIUS && m2.id != member.id)
            {
                switch(member.state)
                {
                case sick:
                    if(m2.state == infected)
                        member.stateCountdown = 7;
                    else if (m2.state == sick)
                    {
                        member.stateCountdown = 7;
                        member.resistance = m2.resistance;
                    }
                    break;

                case infected:
                    if(m2.state == infected)
                        member.resistance -= 1;
                    else if (m2.state == sick && member.resistance < 7)
                        member.changeState(sick);
                    break;

                case recovering:
                    if(m2.state == healthy)
                        m2.resistance += 1;
                    else if(m2.state == sick && member.resistance < 7)
                        member.changeState(sick);
                    else if(m2.state == infected)
                        member.resistance -= 1;
                    break;

                case healthy:
                    if(m2.state == infected && member.resistance < 4)
                        member.changeState(infected);
                    else if(m2.state == sick)
                    {
                        if(member.resistance < 7)
                            member.changeState(infected);
                        else
                            member.resistance -= 3;
                    }
                    else if(m2.state == healthy && m2.resistance>member.resistance)
                        member.resistance = m2.resistance;
                    break;
                case dead:
                    break;
                }
                if(m2.state!=dead)
                    member.direction = glm::normalize(glm::circularRand(1.f));
            }
        }

        //daily resistance change
        float resChange = 0.f;
        switch(member.state)
        {
        case sick:
            resChange = -0.5f;
            break;
        case infected:
            resChange = -0.1f;
            break;
        case recovering:
            resChange = 0.05f;
            break;
        case healthy:
            resChange = 0.1f;
            break;
        case dead:
            break;
        }
        member.resistance += resChange*dt*DAYS_PER_SEC;

        // check if dead and clamp resistance to age
        if(member.resistance<=0)
            member.changeState(dead);
        member.resistance = clamp(0.f, maxAgeResistance(member.age), member.resistance);

        // increment age and countdown
        member.age += DAYS_PER_SEC*dt;
        member.stateCountdown -= DAYS_PER_SEC*dt;

        // evalState
        member.evalState();

        // move
        if(member.state!=dead)
            member.pos += member.direction*member.speed*DAYS_PER_SEC*dt;

    }
}

void TestContagionSim::draw()
{
    vec2 winSize = App::getWindowSize();
    activeCamera();
    for(auto& member : population)
    {
        auto pos = vec2(mapToRange({0, ARENA_X}, {0, winSize.x}, member.pos.x),
                        mapToRange({0, ARENA_Y}, {0, winSize.y}, member.pos.y));
        vec4 color;
        switch(member.state)
        {
        case sick:
            color = {0.933, 0.439, 0.431, 1};
            break;
        case infected:
            color = {0.945, 0.910, 0.433, 1};
            break;
        case recovering:
            color = {0.925, 0.580, 0.431, 1};
            break;
        case healthy:
            color = {0.429, 0.898, 0.476, 1};
            break;
        case dead:
            color = {0, 0, 0, 1};
            break;
        }
        BatchRenderer::drawCircle(pos, 3, 10, color);
    }
    BatchRenderer::end();
}

void TestContagionSim::onStart()
{
    population.clear();
    for(uint i=0; i<POP_COUNT; i++)
    {
        PopMember member;
        member.id = i;
        member.pos.x = glm::linearRand<float>(0, ARENA_X);
        member.pos.y = glm::linearRand<float>(0, ARENA_Y);
        member.direction = glm::normalize(glm::circularRand(1.f));
        member.speed = glm::linearRand(0.f, 10.f);
        member.age = glm::linearRand<uint>(1, 99);
        member.resistance = resistanceRoulette(member.age);
        member.state = healthy;
        member.stateCountdown = 0;
        population.push_back(member);
    }
}

void TestContagionSim::onUpdate(float dt)
{
    if(App::getMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT))
    {
        vec2 winSize = App::getWindowSize();
        auto mousePos = App::getMousePos();
        vec2 mappedMousePos = {mapToRange({0, winSize.x}, {0, ARENA_X}, mousePos.x),
                               mapToRange({0, winSize.y}, {0, ARENA_Y}, mousePos.y)};
        for(auto& member : population)
            if(glm::distance(member.pos, mappedMousePos)<5.f && member.state == healthy)
                member.changeState(infected);

    }
    simulate(dt);
    draw();
}


