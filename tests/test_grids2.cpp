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

    cout << U1 << endl;
    cout << V1 << endl;
    cout << A1 << endl;
   
    return 0;
}