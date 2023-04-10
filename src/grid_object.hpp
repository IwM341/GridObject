#ifndef GRID_OBJECT_HPP
#define GRID_OBJECT_HPP

#include "multigrid.hpp"
#include "object_serialization.hpp"
#include "linear_interpolator.hpp"
#include "container_shift.hpp"
namespace grob{

template <typename GridType,typename ContainerType>
struct GridObject{
    GridType Grid;
    ContainerType Values;

    constexpr static size_t Dim = std::decay<GridType>::Dim;
    typedef typename std::decay<decltye(std::declval<typename std::decay<ContainerType>::type>()[std::declval<size_t>()])>::type value_type;

    GridObject(GridType Grid,ContainerType Values):
        Grid(std::forward<GridType>(Grid)), Values(std::forward<GridType>(Grid)){}

    inline decltype(auto) operator [](const MultiIndex<Dim> &Index) noexcept(noexcept(Values[0])){
        return Values[Grid.LinearIndex(Index)];
    }

    inline const decltype(auto) operator [](const MultiIndex<Dim> &Index) const noexcept(noexcept(Values[0])){
        return Values[Grid.LinearIndex(Index)];
    }

    template <typename Serializer>
    auto Serialize(Serializer && S) const{
        return S.MakeDict(2,
            [](size_t i){return !i ? "Grid" : "Values";},
            [this](size_t i){return !i ? stools::Serialize(Grid,S) : stools::Serialize(Values,S);}
        )
    }
    template <typename Writer>
    void write(Writer && w){
        stools::write(Grid,w);
        stools::write(Values,w);
    }

    template <typename FuncType>
    void map(FuncType && f) noexcept{
        for(MultiIndex<Dim> MI, size_t i; MI != Grid.MultiSize();++i,Grid.MultiIncrement(MI)){
            Values[i] = FuncType(Grid[MI]);
        }
    }
};


template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t);

namespace apply_detail {
template <class F, class Tuple, std::size_t... I>
inline constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
{
  return f(std::get<I>(std::forward<Tuple>(t))...);
  // Note: std::invoke is a C++17 feature
}
}; // namespace detail
 
template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
    return apply_detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{});
}


template <typename Interpolator,typename GridType,typename ContainerType>
struct GridFunction:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    using GOBase::GOBase;
    inline constexpr GridFunction(GOBase go) noexcept:GOBase(std::move(go)){}
    
    template <typename...Args>
    inline auto operator(Args...args) const noexcept{
        return Interpolator::interpolate(Grid,Values,args...);
    }
    template <typename NewInterpolator  = Interpolator,size_t N>
    auto inner_slice(const MultiIndex<N> & MI){
        return GridFunction<NewInterpolator,decltype(Grid.inner(MI)) &,decltype(make_slice(Values,0,0))>(
            Grid.inner(MI),make_slice(Values,Grid.LinearIndex(MI),Grid.inner(MI).size())
        );
    }

    template <typename FuncType>
    void map_pack(FuncType && f) noexcept{
        for(auto MI = Grid.Multi0(), size_t i=0; MI != Grid.MultiSize();++i,Grid.MultiIncrement(MI)){
            Values[i] = apply(FuncType,Grid[MI]);
        }
    }
};


template <typename Interpolator = linear_interpolator,typename GridType,typename LambdaType>
auto make_function_tuple_call(GridType && Grid,LambdaType && Func){
    std::vector<typename std::decay<decltype(Func(Grid[std::declval<MultiIndex<std::decay<GridType>::type::Dim>>]))>::type> values;
    values.reserve(Grid.size());
    for(auto MI = Grid.Multi0;MI != Grid.MultiSize();Grid.MultiIncrement(MI)){
        values.push_back(Func(Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,values::value_type>(std::foeward<GridType>(Grid),std::move(values));
}

template <typename Interpolator = linear_interpolator,typename GridType,typename LambdaType>
auto make_function_pack_call(GridType && Grid,LambdaType && Func){
    std::vector<typename std::decay<decltype(Func(Grid[std::declval<MultiIndex<std::decay<GridType>::type::Dim>>]))>::type> values;
    values.reserve(Grid.size());
    for(auto MI = Grid.Multi0;MI != Grid.MultiSize();Grid.MultiIncrement(MI)){
        values.push_back(apply(Func,Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,values::value_type>(std::foeward<GridType>(Grid),std::move(values));
}



template <typename GridType,typename ContainerType>
struct Histogramm:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    using GOBase::GOBase;
    inline constexpr Histogramm(GOBase go) noexcept:GOBase(std::move(go)){}

    template <size_t N>
    auto inner_slice(const MultiIndex<N> & MI){
        return Histogramm<NewInterpolator,decltype(Grid.inner(MI)) &,decltype(make_slice(Values,0,0))>(
            Grid.inner(MI),make_slice(Values,Grid.LinearIndex(MI),Grid.inner(MI).size())
        );
    }

    template <typename T,typename Arg0,typename...Args>
    inline bool put(T const &value,Arg0 const & arg0,Args const&...args) noexcept{
        static_assert(std::tuple_size<std::tuple<Arg0,Args...>>::value == Dim,"")
        return put_tuple(std::make_tuple(arg0,args...));
    }
    template <typename T,typename Arg>
    inline bool put(T const &value,Arg const& arg) noexcept{
        static_assert(std::tuple_size<std::tuple<Arg0>>::value == Dim,"")
        auto bMI = Grid.spos(arg);
    }

    template <size_t tuple_shift = 0,typename T,typename...Args>
    inline bool put_tuple(T const& value,std::tuple<Args...> const & X)noexcept{
        auto bMI = Grid.spos(X);
        if(std::get<0>(bMI)){
            return false;
        }
        else{
            (*this)[std::get<1>(bMI)] += value;
        }
    }
};



};

#endif