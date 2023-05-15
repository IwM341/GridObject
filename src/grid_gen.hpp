#ifndef GRID_GEN_HPP
#define GRID_GEN_HPP

#include "grid.hpp"

namespace grob {

    template <typename T,typename DensFunc>
    inline auto density_grid (T a,T b,DensFunc && rho,size_t N,size_t iter_num = 10,size_t integr_prec = 20){
    std::vecotr<T> grid(N)
    grid[0] = 0;

    std::vector<T> X0tiks = Vector(N,[N](size_t i){return i/(N-1.0);});
    std::vector<T> X1tiks(N);
    X1tiks[0] = X0tiks[0] = 0;

    for(size_t iter = 0;iter<iter_num;++iter){
//        PVAR(X0tiks);
        for(size_t i=1;i<N;++i){
            double a_x = X0tiks[i-1];
            double b_x = X0tiks[i];
            X1tiks[i] = X1tiks[i-1] + integrateAB([&rho,a,b,a_x,b_x](double t){
                return 1.0/rho(a + (b-a)*(a_x + t*(b_x-a_x)));
            },0,1,integr_prec);
        }
        X1tiks *=(1.0/X1tiks.back());
        std::swap(X0tiks,X1tiks);
    }
    return grob::GridVector<T>(a + (b-a)*X0tiks);
}

};


#endif//GRID_GEN_HPP