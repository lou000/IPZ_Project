#include "testpendulum.h"
#include <Eigen>
#include <Core>
namespace csv::internals {
using std::to_string;
}
#include "csv.hpp"

using namespace Eigen;

MatrixXd matFromCsv(const std::string& path)
{
    csv::CSVFormat format;
    format.no_header();
    csv::CSVReader reader(path, format);
    MatrixXd mat;

    size_t maxRowSize = 0;
    size_t rowNum = 0;
    bool first = true;
    for(auto& row : reader)
    {
        size_t col = 0;
        if(first)
        {
            maxRowSize = row.size();
            mat.resize(1000, maxRowSize);
            first = false;
        }
        for(auto& field : row)
        {
            if(col > maxRowSize)
            {
                maxRowSize = col;
                mat.conservativeResize(1000, maxRowSize);
            }
            mat(rowNum, col) = field.get<double>();
            col++;
        }
        if(col<maxRowSize)
            for(size_t i=1; i<=col-maxRowSize; i++)
            {
                mat(rowNum, col+i) = 0;
            }
        rowNum++;
        if (rowNum % 1000 == 0)
        {
            mat.conservativeResize(rowNum+1000, maxRowSize);
        }
    }
    mat.conservativeResize(rowNum, maxRowSize);
    return mat;
}

double TestPendulum::d2x(double x1, double y1, double x0, double y0)
{
    Array4d arr = {x1, y1, x0, y0};
    return (arr.transpose()*A1.row(0).array()).sum();
}

double TestPendulum::d2y(double x1, double y1, double x0, double y0)
{
    Array4d arr = {x1, y1, x0, y0};
    return (arr.transpose()*A1.row(1).array()).sum();
}

void TestPendulum::reset()
{
    stop = 500;
    x0 = 60;
    y0 = 0;
    x1 = 0;
    y1 = 0;
    x2 = d2x(x1, y1, x0, y0);
    y2 = d2y(x1, y1, x0, y0);
    prevLinePoint = {x0, 0, y0};
    graph.clearLines();
}

TestPendulum::TestPendulum()
{
    ball = MeshRenderer::createCubeSphere(10);
    graph.setPointSize(8);

    // Import data from CSV files
    auto data1 = matFromCsv("../assets/misc/data1.csv");
    auto data2 = matFromCsv("../assets/misc/data2.csv");

    // Separate matrices
    auto data1X = data1.block(0, 0, data1.rows(), 2).transpose();
    auto data1Z = data1.block(0, 2, data1.rows(), 4).transpose();

    auto data2X = data2.block(0, 0, data2.rows(), 2).transpose();
    auto data2Z = data2.block(0, 2, data2.rows(), 4).transpose();

    // Make [Z;Z^2] matrix
    auto Zsq = pow(data2Z.array(), 2);
    MatrixXd ZAndZsq(Zsq.rows()*2, Zsq.cols());
    ZAndZsq.block(0,0,Zsq.rows(), Zsq.cols()) = data2Z;
    ZAndZsq.block(Zsq.rows(),0,Zsq.rows(), Zsq.cols()) = Zsq;

    // Calculate coefficiants for linear and sqr equations
    A1 = (data2X*data2Z.transpose())*(data2Z*data2Z.transpose()).inverse();
    A2 = (data2X*ZAndZsq.transpose())*(ZAndZsq*ZAndZsq.transpose()).inverse();



    // Calculate MSE1
    vec3 prevPoint = {0,0,0};
    bool first = true;
    double MSEx1 = 0;
    double MSEy1 = 0;
    for(int i=0; i<data1X.cols(); i++)
    {
        auto ddxTrue = data1X(0, i);
        auto ddxComp = (data1Z.col(i).transpose().array() * A1.row(0).array()).sum();
        MSEx1 += (ddxTrue-ddxComp)*(ddxTrue-ddxComp);

        auto ddyTrue = data1X(1, i);
        auto ddyComp = (data1Z.col(i).transpose().array() * A1.row(1).array()).sum();
        MSEy1 += (ddyTrue-ddyComp)*(ddyTrue-ddyComp);

        // Add lines from data1 to graph
        vec3 point = {data1Z(2, i), 0, data1Z(3, i)};
        if(!first)
        {
            if(glm::length(prevPoint-point)>1) // only draw line if the length > 1
            {
                graph2.addLine(prevPoint, point, {0, 1, 0, 1});
                prevPoint = point;
            }
        }
        else
        {
            prevPoint = point;
            first = false;
        }
    }
    MSE1 = (MSEx1+MSEy1)/(data1X.cols()*2);

    // Calculate MSE2
    double MSEx2 = 0;
    double MSEy2 = 0;
    auto Zsq1 = pow(data1Z.array(), 2);
    MatrixXd ZAndZsq1(Zsq1.rows()*2, Zsq1.cols());
    ZAndZsq1.block(0,0,Zsq1.rows(), Zsq1.cols()) = data1Z;
    ZAndZsq1.block(Zsq1.rows(),0,Zsq1.rows(), Zsq1.cols()) = Zsq1;

    for(int i=0; i<data1X.cols(); i++)
    {

        auto ddxTrue = data1X(0, i);
        auto ddxComp = (ZAndZsq1.col(i).transpose().array() * A2.row(0).array()).sum();
        MSEx2 += (ddxTrue-ddxComp)*(ddxTrue-ddxComp);

        auto ddyTrue = data1X(1, i);
        auto ddyComp = (ZAndZsq1.col(i).transpose().array() * A2.row(1).array()).sum();
        MSEy2 += (ddyTrue-ddyComp)*(ddyTrue-ddyComp);
//        std::cout<<"MSEx2 += ("<<ddxTrue<<" - "<<ddxComp<<") ^2\n";
//        std::cout<<"MSEy2 += ("<<ddyTrue<<" - "<<ddyComp<<") ^2\n";
    }
    MSE2 = (MSEx2+MSEy2)/(data1X.cols()*2);
}

void TestPendulum::onStart()
{
    GraphicsContext::setClearColor({0.184f, 0.200f, 0.329f, 1.f});
    auto camera = GraphicsContext::getCamera();
    camera->setFov(50.f);
    camera->setPosition({0 ,50.f,50});
    camera->setFocusPoint({0,0,0});
    reset();
    std::cout<<"A1:\n"<<A1<<"\n\n";
    std::cout<<"A2:\n"<<A2<<"\n\n\n";
    std::cout<<"MSE1: "<<MSE1<<"\n";
    std::cout<<"MSE2: "<<MSE2<<"\n";
}

void TestPendulum::onUpdate(float dt)
{
    // Controls
    if(App::getKeyOnce(GLFW_KEY_KP_SUBTRACT))
        dtMulti -= 1;
    if(App::getKeyOnce(GLFW_KEY_KP_ADD))
        dtMulti += 1;
    if(App::getKeyOnce(GLFW_KEY_SPACE))
        reset();

    // Clear ball and its line
    graph.clearPoints();
    graph.removeLastLine();

    // Simulate pendulum
    double timeToSim = dt*dtMulti;
    if(stop>0)
    {
        uint simSteps = (uint)(timeToSim/h);
        for(uint i=0; i<simSteps; i++)
        {
            x0 = x0 + h*x1;
            x1 = x1 + h*x2;

            y0 = y0 + h*y1;
            y1 = y1 + h*y2;

            x2 = d2x(x1, y1, x0, y0);
            y2 = d2y(x1, y1, x0, y0);
        }
        stop-=timeToSim;
    }
    vec3 point = {x0, 0, y0};
    if(glm::length(prevLinePoint-point)>1)
    {
        graph.addLine(prevLinePoint, point, {1,0,0,1});
        prevLinePoint = point;
    }

    // Draw
    float lh = 90;
    vec3 lv = point - vec3{0, 100, 0};
    point.y = 100-sqrt(lh*lh - lv.x*lv.x - lv.z*lv.z);
    graph.addPoints(&point, 1, {1,0,0,1});
    graph.addLine({0, 100, 0}, point, {1,1,1,1});
    graph.draw ({-22,0,0},dt);
    graph2.draw({ 2,0,0},dt);
}
