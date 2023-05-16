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
    grob::GridArray<double,11> A1(2,3);
    auto F1 = grob::make_func_grid1(2.,3.,11,[](double x){return -1/x;},[](double x){return -1/x;});
    cout << U1 << endl;
    cout << V1 << endl;
    cout << A1 << endl;
    cout << F1 << endl;

    auto HU1 = grob::make_histo_grid(U1);
    auto HV1 = grob::make_histo_grid(V1);
    auto HA1 = grob::make_histo_grid(A1);
    auto HF1 = grob::make_histo_grid(F1);

    PVAR(HU1[2]);
    PVAR(HV1[2]);
    PVAR(HA1[2]);
    PVAR(HF1[2]); 

    auto 
   
    return 0;
}