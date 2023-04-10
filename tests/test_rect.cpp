//#include "../src/grid.hpp"
#include "../src/rectangle.hpp"
#include "../src/serialization.hpp"
#include <iostream>
#include "debug_defs.hpp"
#include <vector>
#include <array>

using namespace std;

int main(){
    grob::Rect<float> R(1,2);
    stools::SerializatorJson S(std::cout);

    PVAR(R);
    PVAR(R.volume());

    PVAR((R.get<0,0>()));
    PVAR((R.get<0,1>()));

    auto R2 = grob::make_rect(grob::Rect<float>(0,1),grob::Rect<float>(2,3));
    auto R3 = grob::make_rect(grob::Rect<float>(0,1),grob::Rect<float>(2,3),grob::Rect<float>(1.5,2.5));

    PVAR(R2);
    PVAR(R3);

    PVAR(R2.volume());
    PVAR(R3.volume());

    PVAR((R2.get<1,0b00>()));
    PVAR((R2.get<1,0b01>()));
    PVAR((R2.get<1,0b10>()));
    PVAR((R2.get<1,0b11>()));

    PVAR((R3.get<1,0b00>()));
    PVAR((R3.get<1,0b01>()));
    PVAR((R3.get<1,0b10>()));
    PVAR((R3.get<1,0b11>()));

    cout << "R = ";
    R.Serialize(S);
    cout << endl;
    cout << "R2 = ";
    R2.Serialize(S);
    cout << endl;
    cout << "R3 = ";
    R3.Serialize(S);
    cout << endl;

    return 0;
}