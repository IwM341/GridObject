#include <iostream>
#include "../include/grob/container_shift.hpp"
#include "../include/grob/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>
#include <typeinfo>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace grob;
using namespace std;




int main(){
    grob::GridUniform<double> U1(2,3,11);
    grob::GridVector<double> V1(2,3,11);
    grob::GridArray<double,11> A1(2,3,11);
    auto F1 = grob::make_func_grid(2.,3.,11,[](double x){return -1/x;},[](double x){return -1/x;});
    cout << U1 << endl;
    cout << V1 << endl;
    cout << A1 << endl;
    cout << F1 << endl;

    grob::GridUniformHisto<double> HU1 = U1;
    grob::GridVectorHisto<double> HV1 =V1;
    grob::GridArrayHisto<double,11> HA1 = A1.container();
    auto HF1 =grob::make_grid1<typename decltype(F1)::helper_t>(F1);

    TEST(U1.pos(2.24),2);
    TEST(V1.pos(2.24),2);
    TEST(A1.pos(2.24),2);
    TEST(F1.pos(2.24),3);

    
    TEST(HU1.pos(2.24),2);
    TEST(HV1.pos(2.24),2);
    TEST(HA1.pos(2.24),2);
    TEST(HF1.pos(2.24),3);

    TEST(std::get<1>(U1.spos(2.24)),2);
    TEST(std::get<1>(V1.spos(2.24)),2);
    TEST(std::get<1>(A1.spos(2.24)),2);
    TEST(std::get<1>(F1.spos(2.24)),3);

   
    return 0;
}