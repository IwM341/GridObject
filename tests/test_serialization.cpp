#include <iostream>
#include "../src/container_shift.hpp"
#include "../src/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>
#include "../src/multigrid.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace grob;
using namespace std;

int main(){

    grob::GridUniform<double> U1(2,3,11);
    grob::GridVector<double> V1(2,3,11);
    grob::GridArray<double,11> A1(2,3);

    auto func = [](double x){return -1/x;};   

    grob::GridFunctional<double,decltype(func ),decltype(func )>F1 (2.,3.,11,func ,func ) ;

    

    boost::property_tree::ptree P;
    stools::PtreeSerializator<boost::property_tree::ptree> SP{};

    stools::Serialize(0,SP);


    PVAR(TypeToString(decltype(grob::GridUniform<double>::DeSerialize(P,SP))));
    cout << boolalpha;
    cout << stools::_serialize_impl::is_simply_deserializable<grob::GridUniform<double>,decltype(P),decltype(SP)>::value << endl;
    //PVAR(TypeToString(decltype(simple_deserializable_check(U1,P,SP))));
    //assert(false);


    auto pU1 = stools::Serialize(U1,SP);
    cout << "Uniform: " << TypeToString(decltype(pU1)) << endl;
    boost::property_tree::write_json(cout,pU1);

    std::stringstream MG("{\"size\":11,\"a\":2,\"b\":3}");
    boost::property_tree::ptree U1P;
    boost::property_tree::read_json(MG,U1P);
    auto SU1 = stools::DeSerialize<grob::GridUniform<double>>(U1P,SP);
    cout << SU1 << endl;
    
    
    auto pV1 = stools::Serialize(V1,SP);
    cout << "Vector: " << TypeToString(decltype(pV1)) << endl;
    boost::property_tree::write_json(cout,pV1);

    std::stringstream SG1("[1,2,3,4,5,6,7,8,9,10]");
    boost::property_tree::ptree V1P;
    boost::property_tree::read_json(SG1,V1P);
    auto SV1 = stools::DeSerialize<grob::GridVector<double>>(V1P,SP);
    cout << SV1 << endl;
    

    
    auto pA1 = stools::Serialize(A1,SP);
    cout << "Array: " << TypeToString(decltype(pA1)) << endl;
    boost::property_tree::write_json(cout,pA1);


    std::stringstream SG2("[1,2,3,4,5,6,7,8,9,10]");
    boost::property_tree::ptree A1P;
    boost::property_tree::read_json(SG2,A1P);
    auto SA1 = stools::DeSerialize<grob::GridArray<double,10>>(A1P,SP);
    cout << SA1 << endl;
    //
    auto pF1 = stools::Serialize(F1,SP);
    cout << TypeToString(decltype(pF1)) << endl;
    boost::property_tree::write_json(cout,pF1);

    auto MG2 = grob::mesh_grids(V1,U1);
    auto PMG = stools::Serialize(MG2,SP);
    std::stringstream Rstr;
    boost::property_tree::write_json(Rstr,PMG);
    cout << Rstr.str() <<endl;

    
    boost::property_tree::ptree P3;
    boost::property_tree::read_json(Rstr,P3);
    auto MG2_recover = stools::DeSerialize<decltype(MG2)>(P3,SP);
    cout << MG2_recover << endl;

    print("deserialization of histogrids");

    boost::property_tree::ptree PHU;
    std::stringstream str_phu("{\"size\":11,\"a\":-1,\"b\":1}");
    boost::property_tree::read_json(str_phu,PHU);
    
    //auto GHU = grob::GridUniformHisto<double>::DeSerialize(PHU,SP);
    auto GHU = grob::GridUniformHisto<double>::DeSerialize(PHU,SP);
    cout << GHU << endl;
    cout << GHU[1] << endl;


    return 0;
}