#include <iostream>
#include "../src/container_shift.hpp"
#include "../src/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace grob;
using namespace std;




int main(){
    grob::GridUniform<double> U1(2,3,11);
    grob::GridVector<double> V1(2,3,11);
    grob::GridArray<double,11> A1(2,3);

    auto func = [](double x){return -1/x;};   

    grob::GridFunctional<double,decltype(func ),decltype(func )>F1 (2.,3.,11,func ,func ) ;
    

    grob::GridUniformHisto<double> HU1(2,3,11);
    grob::GridVectorHisto<double> HV1(2,3,11);
    grob::GridArrayHisto<double,11> HA1(2,3);
    grob::GridFunctionalHisto<double,decltype(func ),decltype(func )> HF1 (2.,3.,11,func ,func ) ;
    


    std::cout << U1 <<std::endl;
    std::cout << V1 <<std::endl;
    std::cout << A1 <<std::endl;
    std::cout << F1 <<std::endl;



    std::cout << typeid(decltype(U1)::value_type).name() <<std::endl;

    print(U1.pos(2.34),", ",HU1[U1.pos(2.34)]);
    print(V1.pos(2.34),", ",HV1[V1.pos(2.34)]);
    print(A1.pos(2.34),", ",HA1[A1.pos(2.34)]);
    print(F1.pos(2.34),", ",HF1[F1.pos(2.34)]);

    print(U1.pos(2.34),", ",grob::HistoCast(U1)[U1.pos(2.34)]);
    print(V1.pos(2.34),", ",grob::HistoCast(V1)[V1.pos(2.34)]);
    print(A1.pos(2.34),", ",grob::HistoCast(A1)[A1.pos(2.34)]);
    print(F1.pos(2.34),", ",grob::HistoCast(F1)[F1.pos(2.34)]);


    stools::SerializatorJson S(std::cout);

    cout << "\n\nSerialization:" <<endl;
    U1.Serialize(S);
    cout << "\n";
    F1.Serialize(S);
    cout << "\n";
    A1.Serialize(S);
    cout << "\n";
    V1.Serialize(S);

    
    
    #define ITER(G) cout << "iter "#G": ";\
    for(auto x : G){\
        cout << x << ", ";\
    }\
    cout <<endl;
    
    cout << "\n\nIteration:" <<endl;
    ITER(U1);
    ITER(V1);
    ITER(F1);
    ITER(A1);

    ITER(HU1);
    ITER(HV1);
    ITER(HF1);
    ITER(HA1);

    return 0;
}