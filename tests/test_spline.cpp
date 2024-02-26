#include <iostream>

#include "debug_defs.hpp"

#include "../include/grob/container_shift.hpp"

#include "../include/grob/grid.hpp"

#include "../include/grob/grid_objects.hpp"

#include "../include/grob/multigrid.hpp"
#include <vector>
#include <array>
#include <cmath>


int main(){
    grob::GridVector<float> G = grob::GridUniform<float>(0,1,11);
    std::cout << G << std::endl;

    auto f = [](float x){return 0.5-0.4*x+0.9*x*x+0.9*x*x*x;};
    auto df = [](float x){return -0.4+2*0.9*x + 3*0.9*x*x;};
    auto F = grob::make_function_f<grob::interpolator_spline1D>(G,f);
    COMPARE(F(0.94f),f(0.94f));
    COMPARE(F(0.06f),f(0.06f));
    COMPARE(F(0.48f),f(0.48f));

    auto F1 = grob::make_function_f<grob::interpolator_spline1D_diff>(G,f);

    COMPARE(F1(0.94f),_T(f(0.94f),df(0.94f)));
    COMPARE(F1(0.06f),_T(f(0.06f),df(0.06f)));
    COMPARE(F1(0.48f),_T(f(0.48f),df(0.48f)));

    COMPARE(F1(1.2f),_T(f(1.2f),df(1.2f)));
    COMPARE(F1(-0.2f),_T(f(-0.2f),df(-0.2f)));

    auto GridSqrt = grob::make_func_grid(0.0,1.0,10,[](auto x){return std::sqrt(x);},[](auto x){return x*x;});
    auto BadFunction = grob::make_function_f<grob::linear_interpolator>(GridSqrt,[](auto x){return std::sqrt(x);});
    auto GoodFunction = grob::make_function_f<grob::interpolHidden<grob::linear_interpolator>>(GridSqrt,[](auto x){return std::sqrt(x);});
    
    COMPARE(BadFunction(0.1253),std::sqrt(0.1253));
    COMPARE(BadFunction(0.017453),std::sqrt(0.017453));

    COMPARE(GoodFunction(0.1253),std::sqrt(0.1253));
    COMPARE(GoodFunction(0.017453),std::sqrt(0.017453));

    auto UG = grob::GridUniform<float>(0,1,11);
    auto U2G = grob::mesh_grids(UG,UG);
    auto f2 = [](const auto & P){ 
        auto [x,y] = P;
        return 1 + x + y + x*x+y*y+x*y;
    };
    auto Func2 = grob::make_function_f<grob::interProd<grob::spline1D,grob::spline1D>>(U2G,f2);
    COMPARE(Func2(0.154,0.364),f2(_P(0.154,0.364)));

    return 0;
}