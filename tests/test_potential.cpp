#include <iostream>

#include "debug_defs.hpp"

#include "../include/grob/container_shift.hpp"

#include "../include/grob/grid.hpp"

#include "../include/grob/grid_objects.hpp"

#include "../include/grob/multigrid.hpp"
#include <vector>
#include <array>



int main(){
    grob::GridVector<float> G = grob::GridUniform<float>(0,1,11);
    std::cout << G << std::endl;

    auto f = [](float x){return 0.5-0.4*x+0.9*x*x;};

    auto F = grob::make_function_f<grob::interpolator_spline1D>(G,f);
    COMPARE(F(0.94f),f(0.94f));
    COMPARE(F(0.06f),f(0.06f));
    COMPARE(F(0.48f),f(0.48f));

    return 0;
}