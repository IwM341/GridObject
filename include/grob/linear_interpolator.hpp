#ifndef LINEAR_INTERPOLATOR_HPP
#define LINEAR_INTERPOLATOR_HPP

#include "rectangle.hpp"
#include "templates.hpp"

namespace grob{

    namespace __detail_uniform{
        template <typename T>
        struct self_t{
            typedef T type;
        };

        template <typename T>
        constexpr auto is_uniform_grid(self_t<GridUniform<T>>){
            return std::true_type{};
        } 
        template <typename T>
        constexpr auto is_uniform_grid(T){
            return std::false_type{};
        }

        template <typename T,typename ToHidden,typename FromHdden>
        constexpr auto is_functional_grid(self_t<GridFunctional<T,ToHidden,FromHdden>>){
            return std::true_type{};
        } 
        template <typename T>
        constexpr auto is_functional_grid(T){
            return std::false_type{};
        }


        template <typename GridType,typename Action1,typename Action2> 
        auto apply(std::true_type,GridType && Grid,Action1&& A1,Action2&& A2){
            return std::forward<Action1>(A1)(std::forward<GridType>(Grid));
        }
        template <typename GridType,typename Action1,typename Action2> 
        auto apply(std::false_type,GridType && Grid,Action1&& A1,Action2&& A2){
            return std::forward<Action2>(A2)(std::forward<GridType>(Grid));
        }

        template <typename GridType,typename Action1,typename Action2> 
        inline auto select_if_uniform(GridType && Grid,Action1&& A1,Action2&& A2){
            return apply(is_uniform_grid(self_t<typename std::decay<GridType>::type>{}),
                std::forward<GridType>(Grid),
                std::forward<Action1>(A1),std::forward<Action2>(A2));
        }
        template <typename GridType,typename Action1,typename Action2> 
        inline auto select_if_functional(GridType && Grid,Action1&& TrueAction,Action2&& FalseAction){
            return apply(is_functional_grid(self_t<typename std::decay<GridType>::type>{}),
                std::forward<GridType>(Grid),
                std::forward<Action1>(TrueAction),std::forward<Action2>(FalseAction));
        }


    };

    struct linear_interpolator{
        template <size_t tuple_shift = 0,typename RectType,typename Tuple>
        constexpr inline static  auto get_interpolator_coeffs(RectType const & R,Tuple const & X) noexcept{
            typedef decltype(R.first().interpol_coeff(std::get<tuple_shift>(X))) CoeffType;
            typename templdefs::tuple_cat<CoeffType,
                                decltype(get_interpolator_coeffs<tuple_shift+1>(R.inner_reduce(std::declval<CoeffType>()),X))>::type Result;
            set_interpolator_coeffs(R,X,Result);
            return Result;
        }

        template <size_t tuple_shift = 0,typename RectType,typename TupleX,typename TupleCoeff>
        constexpr inline static  void set_interpolator_coeffs(RectType const & R,TupleX const & X,TupleCoeff & Coeffs) noexcept{
            auto cf = R.first().interpol_coeff(std::get<tuple_shift>(X));
            std::get<tuple_shift>(Coeffs) = cf;
            set_interpolator_coeffs<tuple_shift+1>(R.inner_reduce(cf),X,Coeffs);
        } 
        template <size_t tuple_shift = 0,typename T,typename TupleX,typename TupleCoeff>
        constexpr inline static  void set_interpolator_coeffs(Rect<T> const & R,TupleX const & X,TupleCoeff & Coeffs) noexcept{
            std::get<tuple_shift>(Coeffs) = R.interpol_coeff(std::get<tuple_shift>(X));
        } 

        template <size_t tuple_shift = 0,typename T,typename Tuple>
        constexpr inline static auto get_interpolator_coeffs(Rect<T> const &R, Tuple const & x)noexcept{
            static_assert(std::tuple_size<Tuple>::value != 0,"tuple size of x = 0");
            return std::make_tuple(R.interpol_coeff(std::get<tuple_shift>(x)));
        }


        template <size_t tuple_cf_id=0,typename GridType,typename ContainerType,typename IndexType,typename Tuple>
        constexpr inline static auto _interpolate(GridType const & grid,ContainerType const& Values,size_t v_shift,
                                            IndexType const& Id,Tuple const & Coeffs) noexcept{
            auto cf = std::get<tuple_cf_id>(Coeffs);
            return cf*_interpolate<tuple_cf_id+1>(grid.inner(Id.left()),Values,v_shift+grid.LinearIndex(Id.left()),Id.inner_left(),Coeffs)+
                    (1-cf)*_interpolate<tuple_cf_id+1>(grid.inner(Id.right()),Values,v_shift+grid.LinearIndex(Id.right()),Id.inner_right(),Coeffs);
        }
        template <size_t tuple_cf_id=0,typename GridType,typename ContainerType,typename Tuple>
        constexpr inline static auto _interpolate(GridType const & grid,ContainerType const& Values,size_t v_shift,
                                            Rect<size_t> const& Id,Tuple const & Coeffs) noexcept{
            auto cf = std::get<tuple_cf_id>(Coeffs);
            return cf*Values[v_shift+Id.left]+(1-cf)*Values[v_shift+Id.right];
        }

        template <typename GridType,typename ContainerType,typename Tuple,typename ElementType>
        inline constexpr static auto _extrapolate(GridType const & grid,ContainerType const& Values,
                        Tuple const & X,ElementType const& El) noexcept{
            auto Cf = get_interpolator_coeffs(std::get<1>(El),X);
            return _interpolate(grid,Values,0,std::get<0>(El),Cf);
        }
        /*
        template <typename GridType,typename ContainerType,typename Tuple>
        inline constexpr static auto interpolate(GridType const & grid,ContainerType const& Values,
            Tuple const & X) noexcept ->decltype(_extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)))){
            auto Element = grid.FindElement(templdefs::tuple_reduce(X));
            if(!std::get<1>(Element).contains_tuple(X)){
                return 0;
            }
            return _extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)));
            
        }*/

        template <typename GridType,typename ContainerType,typename Point_t>
        inline constexpr static auto interpolate(GridType const & grid,
                                            ContainerType const & values,
                                            Point_t const & point){
            using namespace __detail_uniform;
            size_t i = grid.pos(point);
            auto h_inv = select_if_uniform(grid,
                [&](auto const & grid){return grid.h_inv();},[&](auto const & grid){return 1/(grid[i+1]-grid[i]);});
            auto u = (point-grid[i])*h_inv;
            
            return u*values[i+1] + (1-u)*values[i];
        }
    };

    struct linear_extrapolator:public linear_interpolator{
        template <typename GridType,typename ContainerType,typename Tuple>
        inline constexpr static auto interpolate(GridType const & grid,ContainerType const& Values,
            Tuple const & X) noexcept ->decltype(_extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)))){
            auto Element = grid.FindElement(templdefs::tuple_reduce(X));
            return _extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)));

        }
    };


    /// @brief cubic interpolation policy
    struct interpolator_spline1D{
        template <typename GridType,typename ContainerType,typename Point_t>
        inline constexpr static auto interpolate(GridType const & grid,
                                            ContainerType const & values,
                                            Point_t const & point){
            
            using namespace __detail_uniform;
            if(grid.size() == 2){
                auto h_inv = select_if_uniform(grid,
                    [&](auto const & grid){return grid.h_inv();},[&](auto const & grid){return 1/(grid[1]-grid[0]);});

                auto u = (point - grid.front())*h_inv;
                return u*values[1] + (1-u)*values[0];
            }
            size_t i = grid.pos(point);
            auto h_inv = select_if_uniform(grid,
                [&](auto const & grid){return grid.h_inv();},[&](auto const & grid){return 1/(grid[i+1]-grid[i]);});
            auto h = select_if_uniform(grid,
                [&](auto const & grid){return grid.h();},[&](auto const & grid){return grid[i+1]-grid[i];});
            auto u = (point-grid[i])*h_inv;
            auto Derivative = [&](size_t i){
                return select_if_uniform(grid,
                    [&](auto const & grid){
                        return (values[i+1]-values[i-1])/2;
                    },
                    [&](auto const & grid){
                        auto h1 = grid[i] - grid[i-1];
                        auto h2 = grid[i+1] - grid[i];
                        auto q = 1/(h1*h2*(h1+h2));
                        auto a1 = h2*h2*q;
                        auto a2 = h1*h1*q;
                        return h*(-a1*values[i-1]+a2*values[i+1] + (a1-a2)*values[i]);
                    }
                );
            };
            if(i == 0){
                auto u_1 = u-1;
                return u*u_1*Derivative(1) + 
                    u_1*u_1*values[0] + u*(2-u)*values[1];
            } else if(i == grid.size()-2){
                auto u_1 = 1-u;
                return u*u_1*Derivative(i) + 
                    u*u*values[i+1] + (1+u)*u_1*values[i];
            } else {
                //Basis functions for splines are: u^2(1-u), u(1-u)^2, u^2(3-2u) (1-u)^2(1+2u)
                auto C_0v = values[i];
                auto C_0d = Derivative(i);
                auto C_1v = values[i+1];
                auto C_1d = Derivative(i+1);

                auto u_q = u*u;
                auto u1 = (u-1);
                auto u1_q = u1*u1;
                auto u2 = 2*u;

                return u1_q*(1+u2)*C_0v + u1_q*u*C_0d + (3-u2)*u_q*C_1v+u1*u_q*C_1d;
            }
        }
    };
    

        /// @brief spline interpolator, evaluating as value ad derivative.
    struct interpolator_spline1D_diff{
        template <typename GridType,typename ContainerType,typename Point_t>
        inline constexpr static auto interpolate(GridType const & grid,
                                            ContainerType const & values,
                                            Point_t const & point){
            
            using namespace __detail_uniform;
            typedef std::decay_t<decltype(values[0])> T;
            typedef std::pair<T,T> ret_type;
            if(grid.size() == 2){
                auto h_inv = select_if_uniform(grid,
                    [&](auto const & grid){return grid.h_inv();},[&](auto const & grid){return 1/(grid[1]-grid[0]);});

                auto u = (point - grid.front())*h_inv;
                return ret_type(u*values[1] + (1-u)*values[0],
                                (values[1]-values[0])*h_inv);
            }
            size_t i = grid.pos(point);
            auto h_inv = select_if_uniform(grid,
                [&](auto const & grid){return grid.h_inv();},[&](auto const & grid){return 1/(grid[i+1]-grid[i]);});
            auto h = select_if_uniform(grid,
                [&](auto const & grid){return grid.h();},[&](auto const & grid){return grid[i+1]-grid[i];});
            auto u = (point-grid[i])*h_inv;
            auto Derivative = [&](size_t i){
                return select_if_uniform(grid,
                    [&](auto const & grid){
                        return (values[i+1]-values[i-1])/2;
                    },
                    [&](auto const & grid){
                        auto h1 = grid[i] - grid[i-1];
                        auto h2 = grid[i+1] - grid[i];
                        auto q = 1/(h1*h2*(h1+h2));
                        auto a1 = h2*h2*q;
                        auto a2 = h1*h1*q;
                        return h*(-a1*values[i-1]+a2*values[i+1] + (a1-a2)*values[i]);
                    }
                );
            };
            if(i == 0){
                auto u_1 = u-1;
                auto Df1 = Derivative(1);
                return ret_type(
                    /*value*/ u*u_1*Df1+ 
                    u_1*u_1*values[0] + u*(2-u)*values[1],
                    /*diff*/h_inv*(u*Df1  + (1-u)*(2*(values[1]-values[0]) - Df1))
                    );
            } else if(i == grid.size()-2){
                auto u_1 = 1-u;
                auto Df0 = Derivative(i);
                return ret_type( u*u_1*Df0 + 
                    /*value*/ u*u*values[i+1] + (1+u)*u_1*values[i],
                    /*diff*/h_inv*(u_1*Df0 + u*(2*(values[i+1]-values[i])-Df0)) );
            } else {
                //Basis functions for splines are: u^2(1-u), u(1-u)^2, u^2(3-2u) (1-u)^2(1+2u)
                auto C_0v = values[i];
                auto C_0d = Derivative(i);
                auto C_1v = values[i+1];
                auto C_1d = Derivative(i+1);

                auto u_q = u*u;
                auto u1 = (u-1);
                auto u1_q = u1*u1;
                auto u2 = 2*u;

                return ret_type(
                    u1_q*(1+u2)*C_0v + u1_q*u*C_0d + (3-u2)*u_q*C_1v+u1*u_q*C_1d,
                    h_inv*((1-u)*C_0d + u*C_1d +3*u*u1*(2*(C_0v-C_1v)+C_0d+C_1d)) 
                );
            }
        }
        
    };

    template <typename Interpolator>
    struct interpolator_functional_grid{
        template <typename GridType,typename ContainerType,typename Point_t>
        inline constexpr static auto interpolate(GridType const & grid,
                                            ContainerType const & values,
                                            Point_t const & point)
        {
            using namespace __detail_uniform;
            return select_if_functional(grid,[&](auto const & _grid){
                    return Interpolator::interpolate(
                        static_cast<const GridUniform<typename GridType::hidden_value_type> &>(_grid.hidden()),
                        values,_grid.to_hidden(point));
                },[&](auto const & _grid){
                    return Interpolator::interpolate(grid,values,point);
                });
        }
    };

    template <typename InterpolatorX,typename InterpolatorY>
    struct interpolator_product{
        template <typename GridType,typename ContainerType,typename Point_t>
        inline constexpr static auto interpolate(GridType const & grid,
                                            ContainerType const & values,
                                            Point_t const & point)
        {
            auto MetaContainerFunction = [&](auto const & Index0){
                return InterpolatorY::interpolate(
                    grid.inner(Index0),
                    make_slice(values,grid.LinearIndex(Index0), grid.inner(Index0).size()),
                    point.tail()
                );
            };
            return InterpolatorX::interpolate(
                grid.grid(),
                as_container(MetaContainerFunction,grid.grid().size()),
                point.template x<0>()
            );

        }
    };

    typedef interpolator_spline1D spline1D;
    typedef interpolator_spline1D_diff splineD1D;

    template <typename interpol_t>
    using  interpolHidden = interpolator_functional_grid<interpol_t>;

    template <typename InterpolatorX,typename InterpolatorY>
    using interProd = interpolator_product<InterpolatorX,InterpolatorY>;
};

#endif
