#include "gamecamera.h"
#include "../Core/utilities.h"

void GameCamera::onUpdate(float dt)
{
    if(animating)
    {
        auto fullTime = timeAccelerating*2+timeFullSpeed;
        auto origDistance = distance(originalPos, targetPos);
        auto nowDistance = distance(m_pos, targetPos);
        auto animPct = 1-(nowDistance/origDistance);
        auto dir = normalize(targetPos - m_pos);
        auto oldDir = normalize(targetPos - originalPos);

        if(animPct<timeAccelerating/fullTime)
        {
//            LOG("Speeding up!\n");
            currentSpeed+=acceleration*dt;
        }
        else if(animPct>(timeAccelerating+timeFullSpeed)/fullTime)
        {
//            LOG("Slowing down!\n");
            currentSpeed-=acceleration*dt;
        }

        auto step = dir*currentSpeed*dt;
//        LOG("dot: %f, animPCT:%f\n", dot(dir, oldDir), animPct);
        if(dot(dir, oldDir)<0.7 || animPct>0.99f)
        {
            m_pos = targetPos;
            currentSpeed = 0;
            animating = false;
        }
        else
            m_pos+=step;
    }
}

void GameCamera::onCreate()
{

}

void GameCamera::doTheFunkyThing(float dt)
{

}

void GameCamera::animateMove(vec3 targetPos)
{
    this->targetPos = targetPos;
    originalPos = m_pos;
    animating = true;
    auto dist = distance(m_pos, targetPos);
    auto distAcc = dist*(timeAccelerating/(timeFullSpeed+timeAccelerating));
    acceleration = (0.5f*distAcc)/(timeAccelerating*timeAccelerating);
    desiredSpeed = acceleration*timeAccelerating;
}
