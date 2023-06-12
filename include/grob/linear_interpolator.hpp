#ifndef LINEAR_INTERPOLATOR_HPP
#define LINEAR_INTERPOLATOR_HPP

#include "rectangle.hpp"
#include "templates.hpp"

namespace grob{
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
        return cf*Values[v_shift+Id.left_]+(1-cf)*Values[v_shift+Id.right_];
    }

    template <typename GridType,typename ContainerType,typename Tuple,typename ElementType>
    inline constexpr static auto _extrapolate(GridType const & grid,ContainerType const& Values,
                    Tuple const & X,ElementType const& El) noexcept{
        auto Cf = get_interpolator_coeffs(std::get<1>(El),X);
        return _interpolate(grid,Values,0,std::get<0>(El),Cf);
    }

    template <typename GridType,typename ContainerType,typename Tuple>
    inline constexpr static auto interpolate(GridType const & grid,ContainerType const& Values,
        Tuple const & X) noexcept ->decltype(_extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)))){
        auto Element = grid.FindElement(templdefs::tuple_reduce(X));
        if(!std::get<1>(Element).contains_tuple(X)){
            return 0;
        }
        return _extrapolate(grid,Values,X,grid.FindElement(templdefs::tuple_reduce(X)));
        
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

};

#endif
