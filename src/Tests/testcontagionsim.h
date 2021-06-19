#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"

class TestContagionSim : public Scene
{
    enum MemberState
    {
        sick        = 0,
        infected    = 1,
        recovering  = 2,
        healthy     = 3,
        dead        = 4
    };

    struct PopMember
    {
        uint id;
        vec2 pos;
        vec2 direction;
        float speed;
        float age;
        float resistance;
        MemberState state;
        float stateCountdown;

        void changeState(MemberState state);
        void evalState();
    };

public:
    TestContagionSim();
    void onStart() override;
    void onUpdate(float dt) override;

private:
    std::vector<PopMember> population;
    void simulate(float dt);
    void draw();
};

