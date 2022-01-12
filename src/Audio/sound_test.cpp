#include<iostream>
#include "AudioListener.h"
#include "AudioSource.h"
#include<thread>
#include <chrono>


void test()
{
    auto listener = AudioListener();
    auto soundtrack = AudioSource("sounds/soundtrack.wav");
    soundtrack.SetGain(8.f);
    soundtrack.Play();
    auto boss_sounds = AudioSource("sounds/boss_steps.wav");
    boss_sounds.setLoop(true);
    boss_sounds.SetGain(4.f);
    auto breathing = AudioSource("sounds/breathing_mono.wav");
    breathing.setLoop(true);
    breathing.SetGain(2.f);
    breathing.Play();
    auto source_1 = AudioSource("sounds/steps2.wav");
    auto scream = AudioSource("sounds/TutPwV_mono.wav");
    scream.SetGain(1.f);
    source_1.setLoop(true);
    source_1.SetGain(3.f);
    float z = -200.f;
    float runner_z = -130.f;
    source_1.UpdatePosition(0.f, 15.f, runner_z);
    boss_sounds.UpdatePosition(0.f, 15.f, z);
    source_1.Play();
    boss_sounds.Play();
    //std::this_thread::sleep_for(std::chrono::seconds(2));

    bool someflag = false;

    while (soundtrack.IsPlaying())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        z += 1.5f;
        runner_z += 1.f;
        if (z > runner_z && !someflag)
        {
            source_1.Stop();
            breathing.Stop();
            boss_sounds.Pause();
            scream.UpdatePosition(1.f, 5.f, runner_z);
            scream.Play();

            someflag = true;
        }
        if (!scream.IsPlaying())
        {
            if (someflag && !boss_sounds.IsPlaying())
            {
                boss_sounds.Play();
            }
            boss_sounds.UpdatePosition(0.f, 5.f, z);
            source_1.UpdatePosition(0.f, 5.f, runner_z);
            breathing.UpdatePosition(1.f, 5.f, runner_z);
        }

    }
    std::cout << z;
}
