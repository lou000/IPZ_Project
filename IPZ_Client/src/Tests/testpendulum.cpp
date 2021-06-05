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

TestPendulum::TestPendulum()
{
    auto data1 = matFromCsv("../assets/misc/data1.csv");
    auto data2 = matFromCsv("../assets/misc/data2.csv");
    data1X = data1.block(0, 0, data1.rows(), 2).transpose();
    data1Z = data1.block(0, 2, data1.rows(), 4).transpose();

    data2X = data2.block(0, 0, data2.rows(), 2).transpose();
    data2Z = data2.block(0, 2, data2.rows(), 4).transpose();

    auto Zsq = pow(data2Z.array(), 2);
    MatrixXd ZAndZsq(Zsq.rows()*2, Zsq.cols());
    ZAndZsq.block(0,0,Zsq.rows(), Zsq.cols()) = data2Z;
    ZAndZsq.block(Zsq.rows(),0,Zsq.rows(), Zsq.cols()) = Zsq;


    A1 = (data2X*data2Z.transpose())*(data2Z*data2Z.transpose()).inverse();
    A2 = (data2X*ZAndZsq.transpose())*(ZAndZsq*ZAndZsq.transpose()).inverse();
    std::cout<<A1<<"\n\n";
    std::cout<<A2<<"\n";
}

void TestPendulum::onStart()
{

}

void TestPendulum::onUpdate(float dt)
{

}
