#ifndef GRID_OBJECT_HPP
#define GRID_OBJECT_HPP

#include "multigrid.hpp"
#include "object_serialization.hpp"
#include "container_shift.hpp"
#include "linear_interpolator.hpp"
namespace grob{


/// @brief Base class for gridfunction/histogramm
/// @tparam GridType anydim grid
/// @tparam ContainerType linear container of values
template <typename GridType,typename ContainerType>
struct GridObject{
    GridType Grid;
    ContainerType Values;

    constexpr static size_t Dim = std::decay<GridType>::type::Dim;
    typedef typename std::decay<decltype(std::declval<typename std::decay<ContainerType>::type>()[std::declval<size_t>()])>::type value_type;


    GridObject(GridType Grid,ContainerType Values):
        Grid(std::forward<GridType>(Grid)), Values(std::forward<ContainerType>(Values)){}

    inline size_t size()const noexcept{
        return Grid.size();
    }

    /// @brief maps Values by grid multiindex 
    /// @param Index 
    /// @return 
    inline decltype(auto) operator [](const MultiIndex<Dim> &Index) noexcept(noexcept(Values[0])){
        return Values[Grid.LinearIndex(Index)];
    }
    
    /// @brief  
    inline decltype(auto) operator [](const MultiIndex<Dim> &Index) const noexcept(noexcept(Values[0])){
        return Values[Grid.LinearIndex(Index)];
    }

    /// @brief 
    template <typename Serializer>
    auto Serialize(Serializer && S) const{
        return S.MakeDict(2,
            [](size_t i){return !i ? "Grid" : "Values";},
            [this,&S](size_t i){return !i ? stools::Serialize(Grid,S) : stools::Serialize(Values,S);}
        );
    }
    /// @brief 
    template <typename Writer>
    void write(Writer && w){
        stools::write(Grid,w);
        stools::write(Values,w);
    }

    /// @brief 
    template <typename Object,typename DeSerializer>
    static GridObject DeSerialize(Object const&Obj,DeSerializer && DS){
        return GridObject(stools::DeSerialize<GridType>(DS.GetProperty(Obj,"Grid"),DS),
                        stools::DeSerialize<ContainerType>(DS.GetProperty(Obj,"Values"),DS));
    }
    /// @brief 
    template <typename Object,typename DeSerializer>
    void init_serialize(Object const& Obj,DeSerializer && DS){
        stools::init_serialize(Grid,DS.GetProperty(Obj,"Grid"));
        stools::init_serialize(Values,DS.GetProperty(Obj,"Values"));
    }

    /// @brief  
    template <typename Reader>
    void init_read(Reader && r){
        stools::init_read(Grid,r);
        stools::init_read(Values,r);
    }
    /// @brief  
    template <typename Reader>
    static GridObject read(Reader && r){
        return GridObject(stools::read<GridType>(r),stools::read<ContainerType>(r));
    }

    /// @brief set Values of index (i1,...in) to f(Grid[(i1,...in)]) 
    /// @param f 
    template <typename FuncType>
    void map(FuncType && f) noexcept{
        size_t i=0;
        for(auto MI = Grid.Multi0(); !Grid.IsEnd(MI);++i,Grid.MultiIncrement(MI)){
            Values[i] = FuncType(Grid[MI]);
        }
    }
};




/// @brief Grid Function Type
/// @tparam Interpolator class, with interpolate(Grid,Values,tuple(x1,..,xn)) method 
/// @tparam GridType 
/// @tparam ContainerType 
template <typename Interpolator,typename GridType,typename ContainerType>
struct GridFunction:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    using GOBase::GOBase;
    inline constexpr GridFunction(GOBase go) noexcept:GOBase(std::move(go)){
        static_assert(!is_histo_grid<typename std::decay<GridType>::type>::value,
                      "histo grid at grid function");
    }
    
    template <typename...Args>
    inline auto operator()(Args const&...args) const noexcept{
        static_assert(std::tuple_size<std::tuple<Args...>>::value == GOBase::Dim,"expect the same number of arguments");
        return Interpolator::interpolate(GOBase::Grid,GOBase::Values,std::make_tuple(args...));
    }
    template <typename...Args>
    inline auto operator()(std::tuple<Args...> const& X) const noexcept{
        static_assert(std::tuple_size<std::tuple<Args...> >::value == GOBase::Dim,"expect the same number of arguments");
        return Interpolator::interpolate(GOBase::Grid,GOBase::Values,X);
    }

    /// @brief give view on inner dim grid function
    /// @tparam NewInterpolator optional new interpolator
    /// @tparam N depth of inner grid
    /// @param MI index of inner grid (e.g. inner_slice(i) -> GridFunction(Grid.InnerGrids[i], Values, shifted on Grid.Grid.size() ))
    /// @return 
    template <typename NewInterpolator = Interpolator,size_t N>
    auto inner_slice(const MultiIndex<N> & MI){
        return GridFunction<NewInterpolator,decltype(GOBase::Grid.inner(MI)) &,decltype(make_slice(GOBase::Values,0,0))>(
            GOBase::Grid.inner(MI),make_slice(GOBase::Values,GOBase::Grid.LinearIndex(MI),GOBase::Grid.inner(MI).size())
        );
    }

    /// @brief same as nap, but f takes x1,...xn, not tuple(x1,...xn) 
    template <typename FuncType>
    void map_pack(FuncType && f) noexcept{
        size_t i=0;
        for(auto MI = GOBase::Grid.Multi0(); MI != GOBase::Grid.MultiSize();++i,GOBase::Grid.MultiIncrement(MI)){
            GOBase::Values[i] = apply(f,GOBase::Grid[MI]);
        }
    }

    INHERIT_SERIALIZATOR(GOBase,GridFunction)
    INHERIT_READ(GOBase,GridFunction)
};

/// @brief make grid function, initializing by Func
/// @param Grid 
/// @param Func object, callable with tuple: Func(tuple(x1,...xn))
template <typename Interpolator =  linear_interpolator,typename GridType,typename LambdaType>
auto make_function_tuple(GridType && Grid,LambdaType && Func){
    typedef typename std::decay<decltype(Func(Grid[std::declval<MultiIndex<std::decay<GridType>::type::Dim>>]))>::type value_type;
    std::vector<value_type> values;
    values.reserve(Grid.size());

    for(auto MI = Grid.Multi0();!Grid.IsEnd(MI);Grid.MultiIncrement(MI)){
        values.push_back(Func(Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,value_type>(std::forward<GridType>(Grid),std::move(values));
}

/// @brief make grid function, initializing by Func
/// @param Grid 
/// @param Func object, callable with pack: Func(x1,...xn)
template <typename Interpolator = linear_interpolator,typename GridType,typename LambdaType>
auto make_function_f(GridType && Grid,LambdaType && Func){
    typedef typename std::decay<decltype(templdefs::apply_tuple(Func,Grid[std::declval<MultiIndex<std::decay<GridType>::type::Dim>>()]))>::type value_type;
    std::vector<value_type> values;
    values.reserve(Grid.size());

    for(auto MI = Grid.Multi0();!Grid.IsEnd(MI);Grid.MultiIncrement(MI)){
        //std::cout <<MI << ", " <<  (Grid[MI]) << std::endl; 
        values.push_back(templdefs::apply_tuple(Func,Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,std::vector<value_type> >
            (std::forward<GridType>(Grid),std::move(values));
}

/// @brief makes GridFunction from grid and vector of values
template <typename Interpolator = linear_interpolator,typename GridType,typename VectorType>
auto make_function(GridType && Grid,VectorType && Values){
    return GridFunction<Interpolator,typename std::decay<GridType>::type,
        typename std::decay<VectorType>::type>(std::forward<GridType>(Grid),std::forward<VectorType>(Values));
}


/// @brief Histogramm class
/// @tparam GridType Grid
/// @tparam ContainerType Values 
template <typename GridType,typename ContainerType>
struct Histogramm:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    using GOBase::GridObject;
     
    inline constexpr Histogramm(GOBase go) noexcept:GOBase(std::move(go)){}

    /// @brief give view on inner dim histo
    /// @tparam NewInterpolator optional new interpolator
    /// @tparam N depth of inner grid
    /// @param MI index of inner grid (e.g. inner_slice(i) -> Histogramm(Grid.InnerGrids[i], Values, shifted on Grid.Grid.size() ))
    /// @return 
    template <size_t N>
    auto inner_slice(const MultiIndex<N> & MI){
        return Histogramm<decltype(GOBase::Grid.inner(MI)) &,decltype(make_slice(GOBase::Values,0,0))>(
            GOBase::Grid.inner(MI),make_slice(GOBase::Values,GOBase::Grid.LinearIndex(MI),GOBase::Grid.inner(MI).size())
        );
    }

    /// @brief puts value into bin, containing (arg0,...args)
    /// @return true in case of sucsess, false if Grid not contains point 
    template <typename T,typename Arg0,typename...Args>
    inline bool put(T const &value,Arg0 const & arg0,Args const&...args) noexcept{
        return put_tuple(value,std::make_tuple(arg0,args...));
    }

    template <typename T,typename Arg>
    inline bool put(T const &value,Arg const& arg) noexcept{
        static_assert(std::tuple_size<std::tuple<Arg>>::value == GOBase::Dim,"numbper of args mismatches dimension");
        auto bMI = GOBase::Grid.spos(arg);
        if(!std::get<0>(bMI))
            return false;
        else{
            (*this)[std::get<1>(bMI)] += value;
            return true;
        }
    }

    template <size_t tuple_shift = 0,typename T,typename...Args>
    inline bool put_tuple(T const& value,std::tuple<Args...> const & X)noexcept{
        auto bMI = GOBase::Grid.spos(X);
        if(!std::get<0>(bMI)){
            return false;
        }
        else{


            (*this)[std::get<1>(bMI)] += value;
            return true;
        }
    }

    INHERIT_SERIALIZATOR(GOBase,Histogramm)
    INHERIT_READ(GOBase,Histogramm)
};


/// @brief make histogramm
/// @param Grid 
/// @param Values 
/// @return 
template <typename GridType,typename VectorType>
auto make_histo(GridType && Grid,VectorType && Values){
    static_assert(is_histo_grid<typename std::decay<GridType>::type>::value,"histogramm can be created only with histo grid");
    return Histogramm<typename std::decay<GridType>::type,
        typename std::decay<VectorType>::type>(std::forward<GridType>(Grid),std::forward<VectorType>(Values));
}

/// @brief make histogramm with container vector<value_type>
/// @tparam value_type 
/// @param Grid 
/// @return 
template <typename value_type,typename GridType>
auto make_histo(GridType && Grid){
    static_assert(is_histo_grid<typename std::decay<GridType>::type>::value,"histogramm can be created only with histo grid");
    std::vector<value_type> values(Grid.size(),0);
    return Histogramm<typename std::decay<GridType>::type,std::vector<value_type>>(std::forward<GridType>(Grid),std::move(values));
}

/// @brief make histogramm, which grid is a referance
/// @param Grid
/// @param Values
/// @return
template <typename GridType,typename VectorType>
auto make_histo_ref(GridType & Grid,VectorType && Values){
    static_assert(is_histo_grid<typename std::decay<GridType>::type>::value,"histogramm can be created only with histo grid");
    return Histogramm<GridType &,
        typename std::decay<VectorType>::type>(Grid,std::forward<VectorType>(Values));
}

/// @brief make histogramm, which grid and values are referances
/// @param Grid
/// @param Values
/// @return
template <typename GridType,typename VectorType>
auto make_histo_ref(GridType & Grid,VectorType & Values){
    static_assert(is_histo_grid<typename std::decay<GridType>::type>::value,"histogramm can be created only with histo grid");
    return Histogramm<GridType &,
        VectorType &>(Grid,Values);
}

/// @brief make histogramm, which grid is a referance, with container vector<value_type>
/// @tparam value_type
/// @param Grid
/// @return
template <typename value_type,typename GridType>
auto make_histo_ref(GridType & Grid){
    static_assert(is_histo_grid<typename std::decay<GridType>::type>::value,"histogramm can be created only with histo grid");
    std::vector<value_type> values(Grid.size(),0);
    return Histogramm<GridType &,std::vector<value_type>>(Grid,std::move(values));
}



};

#endif
