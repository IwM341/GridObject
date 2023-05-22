#include <iostream>

#include "debug_defs.hpp"

#include "../src/container_shift.hpp"

#include "../src/grid.hpp"

#include "../src/multigrid.hpp"
#include <vector>
#include <array>


using namespace std;

int main(){
    grob::GridUniform<float> U(0,1,11);
    std::vector<decltype(U)> VU(11,U) ;
    grob::MultiGrid<decltype(U),decltype(VU)> MG(U,VU);

    
    auto I = MG.pos(0.5,0.5);
    cout << I <<endl;
    cout << MG[I] << endl;

    auto MG1 = grob::make_grid(U,VU);

    //using namespace stools::print_vector; 
    //cout << VU << endl;
    cout << MG1 << endl;

    stools::SerializatorJson S(std::cout);
    //stools::Serialize(MG1,S);

    auto G2 = grob::mesh_grids(U,U);
    cout << G2 <<endl;


    auto G3 = grob::mesh_grids(G2,U);


    cout << G3 << endl;

    print();
    print(G3[G3.pos(0.14,0.54,0.37)]);


    //auto G4 = grob::mesh_grids(G23,U);

    print(G3.FindIndex(0.1,0.5,0.7));
    print(G3.FindElement(0.1,0.5,0.7));
    print("version 1 1 6");
    return 0;
}