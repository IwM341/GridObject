#include <iostream>
#include "../src/container_shift.hpp"
#include "../src/grid.hpp"
#include "../src/grid_objects.hpp"
#include "../src/linear_interpolator.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace grob;
using namespace std;




int main(){
    grob::GridUniform<double> U1(0,10,11);
    grob::GridVector<double> V1(2,3,11);
    grob::GridArray<double,11> A1(2,3);

    auto func = [](double x){return -1/x;};   

    grob::GridFunctional<double,decltype(func ),decltype(func )>F1 (2.,3.,11,func ,func ) ;

    std::vector<float> vals;
    for(size_t i=0;i<U1.size();++i){
        vals.push_back(i);
    }

    grob::GridFunction<grob::linear_interpolator,decltype(U1),std::vector<float>> GO(U1,std::move(vals));

    cout << GO[3] << endl;
    cout << GO(2.3) << endl;

    auto f = [](double x,double y){return 2+3*x+4*y+2*x*y;};
    auto F2 = grob::make_function_f(grob::make_grid_f(
        U1,[](size_t i){
            return grob::GridUniform<float>(0,1,11);
        }),f
    );

    auto F3 = grob::make_function_f(
        grob::mesh_grids(U1,grob::mesh_grids(A1,F1)),
        [](double x,double y,double z){return x+y+z;}
    );

    PVAR(F2.Grid);
    PVAR(F2.Values);

    cout << F2(0.5,0.5) << endl;
    cout << f(0.5,0.5) << endl;

    PVAR(F3.Grid);
    PVAR(F3(1,2,3));

    auto H2 = grob::make_histo<float>(grob::mesh_grids(grob::HistoCast(U1),grob::HistoCast(U1)));
    PVAR(H2.Grid);
    PVAR(H2.Values);
    PVAR(H2.Grid.pos(5.5,4.6));
    H2.put(0.12,4.5,5.5);
    PVAR(H2.Values);
    PVAR(H2[std::make_tuple(4,5)]);

    stools::PtreeSerializator<boost::property_tree::ptree> SP{};
    std::stringstream out;


    auto Pt =  H2.Serialize(SP);
    auto H2r = stools::DeSerialize<decltype(H2)>(Pt,SP);

    PVAR(H2r.Grid);
    PVAR(H2r.Values);
    
    
    return 0;
}