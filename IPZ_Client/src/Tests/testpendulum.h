#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"
#include <Dense>

class TestPendulum : public Scene
{
public:
    TestPendulum();
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;

private:
    Eigen::MatrixXd data1X;
    Eigen::MatrixXd data1Z;
    Eigen::MatrixXd data2X;
    Eigen::MatrixXd data2Z;

    Eigen::MatrixXd A1;
    Eigen::MatrixXd A2;

};

