#ifndef GRID_GEN_HPP
#define GRID_GEN_HPP
#include <vector>
#include "grid.hpp"

namespace grob {

    namespace __detail{
        template <typename T,typename FuncType>			
        auto integrateAB(FuncType f,T a,T b,size_t N){
            
            T sum = 0.0;
            T h = (b-a)/N;
            for (size_t i = 0; i < N; i++) {
                sum+= f(a + (i+(T)0.5)*h)*h;
            }
            return sum;
        }
    };

    /// @brief creates non-uniform vector determined by density function
    /// @param a start
    /// @param b end
    /// @param rho density function with argument in [a,b]
    /// @param N number of points
    /// @param iter_num (optional) number of iteration in creating process
    /// @param integr_prec (optional) precision in integration in creating process
    /// @return 
    template <typename T,typename DensFunc>
    inline auto density_vector(T a,T b,DensFunc && rho,size_t N,size_t iter_num = 10,size_t integr_prec = 20){
        std::vector<T> X0tiks(N);
        for(size_t i=0;i<N;++i){
           X0tiks[i]= i/(N-1.0);
        }
        std::vector<T> X1tiks(N);
        X1tiks[0] = X0tiks[0] = 0;

        for(size_t iter = 0;iter<iter_num;++iter){
    //        PVAR(X0tiks);
            for(size_t i=1;i<N;++i){
                T a_x = X0tiks[i-1];
                T b_x = X0tiks[i];
                X1tiks[i] = X1tiks[i-1] + __detail::integrateAB([&rho,a,b,a_x,b_x](T t){
                    return 1.0/rho(a + (b-a)*(a_x + t*(b_x-a_x)));
                },0,1,integr_prec);
            }
            auto mlt = (1.0/X1tiks.back());
            for(auto & x : X1tiks){
                x *= mlt;
            }
            std::swap(X0tiks,X1tiks);
        }
        for(auto & x : X0tiks){
            x = a + (b-a)*x;
        }
        return X0tiks;
    }

    /// @brief creates non-uniform vector determined by density function
    /// @param a start
    /// @param b end
    /// @param rho density function with argument in [0,1]
    /// @param N number of points
    /// @param iter_num (optional) number of iteration in creating process
    /// @param integr_prec (optional) precision in integration in creating process
    /// @return 
    template <typename T,typename DensFunc>
    inline auto density_vector_nt(T a,T b,DensFunc && rho,size_t N,size_t iter_num = 10,size_t integr_prec = 20){
        std::vector<T> X0tiks(N);
        for(size_t i=0;i<N;++i){
           X0tiks[i]= i/(N-(T)1.0);
        }
        std::vector<T> X1tiks(N);
        X1tiks[0] = X0tiks[0] = 0;

        for(size_t iter = 0;iter<iter_num;++iter){
    //        PVAR(X0tiks);
            for(size_t i=1;i<N;++i){
                T a_x = X0tiks[i-1];
                T b_x = X0tiks[i];
                X1tiks[i] = X1tiks[i-1] + __detail::integrateAB([&rho,a,b,a_x,b_x](T t){
                    return ((T)1)/rho(a_x + t*(b_x-a_x));
                }, (T)0, (T)1,integr_prec);
            }
            auto mlt = (1.0/X1tiks.back());
            for(auto & x : X1tiks){
                x *= mlt;
            }
            std::swap(X0tiks,X1tiks);
        }
        for(auto & x : X0tiks){
            x = a + (b-a)*x;
        }
        return X0tiks;
    }

};


#endif//GRID_GEN_HPP
