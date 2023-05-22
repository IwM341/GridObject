#include <iostream>
#include "../src/container_shift.hpp"
#include "../src/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>
#include <typeinfo>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace grob;
using namespace std;




int main(){
    grob::Grid1<std::vector<double>> U1(2,3,11);
    grob::GridVector<double> V1(2,3,11);
    grob::GridArray<double,11> A1(2,3,11);
    auto F1 = grob::make_func_grid1(2.,3.,11,[](double x){return -1/x;},[](double x){return -1/x;});
    cout << U1 << endl;
    cout << V1 << endl;
    cout << A1 << endl;
    cout << F1 << endl;

    auto HU1 = grob::make_histo_grid(U1);
    auto HV1 = grob::make_histo_grid(V1);
    auto HA1 = grob::make_histo_grid(A1);
    auto HF1 = grob::make_histo_grid(F1);

    TEST(Grid1Cast(HU1)[2],U1[2]);
    TEST(Grid1Cast(HV1)[2],V1[2]);
    TEST(Grid1Cast(HA1)[2],A1[2]);
    TEST(Grid1Cast(HF1)[2],F1[2]); 

    TEST(HistoCast(HU1)[2],HU1[2]);
    TEST(HistoCast(HV1)[2],HV1[2]);
    TEST(HistoCast(HA1)[2],HA1[2]);
    TEST(HistoCast(HF1)[2],HF1[2]);

    TEST(U1.pos(2.24),2);
    TEST(V1.pos(2.24),2);
    TEST(A1.pos(2.24),2);
    TEST(F1.pos(2.24),3);

    TEST(std::get<1>(U1.spos(2.24)),2);
    TEST(std::get<1>(V1.spos(2.24)),2);
    TEST(std::get<1>(A1.spos(2.24)),2);
    TEST(std::get<1>(F1.spos(2.24)),3);

   
    return 0;
}