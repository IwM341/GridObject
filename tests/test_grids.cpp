#include <iostream>
#include "../include/grob/container_shift.hpp"
#include "../include/grob/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>
#include <C:\Users\MainUser\AppData\Local\Programs\Python\Python310\include\Python.h>
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
    
    grob::GridRangeLight<int> RL1(11);
    grob::GridRange<int> R1(4,2,10);

    PyObject_IsInstance(0,0);

    PVAR(U1);
    PVAR(V1);
    PVAR(A1);
    PVAR(F1);
    PVAR(HU1);
    PVAR(HV1);
    PVAR(HA1);
    PVAR(HF1);

    PVAR(RL1);
    PVAR(R1);

    std::cout << "\n********\nprint sizes" << std::endl;
    PVAR(U1.size());
    PVAR(V1.size());
    PVAR(A1.size());
    PVAR(F1.size());
    PVAR(HU1.size());
    PVAR(HV1.size());
    PVAR(HA1.size());
    PVAR(HF1.size());

    PVAR(RL1.size());
    PVAR(R1.size());


    std::cout << typeid(decltype(U1)::value_type).name() <<std::endl;

    print("U1",U1.pos(2.34),", ",HU1[U1.pos(2.34)]);
    print("V1",V1.pos(2.34),", ",HV1[V1.pos(2.34)]);
    print("A1",A1.pos(2.34),", ",HA1[A1.pos(2.34)]);
    print("F1",F1.pos(2.34),", ",HF1[F1.pos(2.34)]);

    print("RL1",RL1.pos(3),", ",RL1[RL1.pos(3)]);
    print("RL1",R1.pos(6),", ",R1[R1.pos(6)]);

    stools::SerializatorJson S(std::cout);
    /*
    cout << "\n\nSerialization:" <<endl;
    U1.Serialize(S);
    cout << "\n";
    F1.Serialize(S);
    cout << "\n";
    A1.Serialize(S);
    cout << "\n";
    V1.Serialize(S);
    */
    
    
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