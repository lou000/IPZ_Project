#pragma once
#include "../Core/application.h"
#include "../Core/scene.h"
#include "../Renderer/graph.h"
#include <Dense>

class TestPendulum : public Scene
{
public:
    TestPendulum();
    virtual void onStart() override;
    virtual void onUpdate(float dt) override;

private:

    double d2x(double x1, double y1, double x0, double y0);
    double d2y(double x1, double y1, double x0, double y0);
    void reset();

    Eigen::MatrixXd A1;
    Eigen::MatrixXd A2;

    double stop = 500;
    double h  = 2e-5; // any less and it could fail on some pcs, longer frame more to sim runaway effect
    double x0 = 60;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    double x2;
    double y2;
    double MSE1;
    double MSE2;

    double dtMulti = 1;
    vec3 prevLinePoint = {x0, 0, y0};
    Graph3d graph  = Graph3d({-60,60}, {0,100}, {-60,60}, 20);
    Graph3d graph2 = Graph3d({-60,60}, {0,100}, {-60,60}, 20);
    std::shared_ptr<Mesh> ball;

};

