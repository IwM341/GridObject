#ifndef GRID_OBJECT_HPP
#define GRID_OBJECT_HPP

#include "multigrid.hpp"
#include "object_serialization.hpp"
#include "container_shift.hpp"
#include "linear_interpolator.hpp"
#include "point.hpp"
namespace grob{


/// @brief Base class for gridfunction/histogramm
/// @tparam GridType anydim grid
/// @tparam ContainerType linear container of values
template <typename GridType,typename ContainerType>
struct GridObject{
    GridType Grid;
    ContainerType Values;

    constexpr static size_t Dim = std::decay<GridType>::type::Dim;
    typedef typename std::decay<
        decltype(
            std::declval<typename std::decay<ContainerType>::type>()
                [std::declval<size_t>()]
        )>::type value_type;

    GridObject(){}
    GridObject(GridType Grid,ContainerType Values):
        Grid(std::forward<GridType>(Grid)), Values(std::forward<ContainerType>(Values)){}

    inline size_t size()const noexcept{
        return Grid.size();
    }

    /// @brief maps Values by grid multiindex 
    /// @param Index 
    /// @return
    template <typename MultiIndex_t = std::decay_t<GridType>::MultiIndexType>
    inline decltype(auto) operator [](const MultiIndex_t &Index) noexcept(noexcept(Values[0])){
        return Values[Grid.LinearIndex(Index)];
    }
    /// @brief
    template <typename MultiIndex_t = std::decay_t<GridType>::MultiIndexType>
    inline decltype(auto) operator [](const MultiIndex_t& Index) const {
        return Values[Grid.LinearIndex(Index)];
    }
    
    typedef decltype(Grid.begin()) Grid_iterator;
    struct iterator : public Grid_iterator{
        GridObject & __gobj;
        iterator( Grid_iterator __it,GridObject & __gobj):
            Grid_iterator(__it),__gobj(__gobj){}
        inline decltype(auto) operator *(){
            auto m_bin = *static_cast<Grid_iterator&>(*this);
            decltype(auto) v_ref = __gobj[this->index()];
            return std::tuple<decltype(m_bin),decltype(v_ref)>(
                m_bin,
                v_ref
                );
        }
    };
    struct const_iterator : public Grid_iterator{
        GridObject const& __gobj;
        const_iterator( Grid_iterator __it,GridObject const& __gobj):
            Grid_iterator(__it),__gobj(__gobj){}
        inline decltype(auto) operator *()const{
            auto m_bin = *static_cast<Grid_iterator&>(*this);
            decltype(auto) v_ref = __gobj[this->index()];
            return std::tuple<decltype(m_bin),decltype(v_ref)>(
                m_bin,
                v_ref
                );
        }
    };
    inline iterator begin() {
        return iterator(Grid.begin(),*this);
    }
    inline iterator end() {
        return iterator(Grid.end(),*this);
    }
    inline const_iterator begin() const{
        return const_iterator(Grid.begin(),*this);
    }
    inline const_iterator end() const{
        return const_iterator(Grid.end(),*this);
    }
    inline const_iterator cbegin() const{
        return const_iterator(Grid.begin(),*this);
    }
    inline const_iterator cend() const{
        return const_iterator(Grid.end(),*this);
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
        for(auto MI = Grid.MultiZero(); !Grid.IsEnd(MI);++i,Grid.MultiIncrement(MI)){
            Values[i] = f(Grid[MI]);
        }
    }
};

/// @brief makes GridFunction from grid and vector of values
/// @param Grid 
/// @param Values 
/// @return 
template <typename GridType,typename VectorType>
GridObject<std::decay_t<GridType>, std::decay_t<VectorType>>
make_grid_object(GridType&& Grid, VectorType&& Values) 
{
    return GridObject<std::decay_t<GridType>,
        std::decay_t<VectorType>>
        (
            std::forward<GridType>(Grid),
            std::forward<VectorType>(Values)
        );
}
template <typename GridType, typename VectorType>
GridObject<GridType, VectorType>
make_grid_object_ref(GridType&& Grid, VectorType&& Values)
{
    return GridObject<GridType, VectorType>(
            Grid,
            Values
        );
}




/// @brief Grid Function Type
/// @tparam Interpolator class, with interpolate(Grid,Values,tuple(x1,..,xn)) method 
/// @tparam GridType 
/// @tparam ContainerType 
template <typename Interpolator,typename GridType,typename ContainerType>
struct GridFunction:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    typedef Interpolator interpolator_t;
    using GOBase::GOBase;
    inline constexpr GridFunction(GOBase go) noexcept:GOBase(std::move(go)){

    }
    
    template <typename Interpol_t = Interpolator,typename...Args>
    auto eval(Args const&...args)const noexcept{
        return Interpolator::interpolate(GOBase::Grid,GOBase::Values,make_point(args...)); 
    }
    template <typename Interpol_t = Interpolator,typename...Args>
    auto eval(Point<Args...> const& X)const noexcept{
        static_assert(sizeof...(Args) == GOBase::Dim,"expect the same number of arguments");
        return Interpolator::interpolate(GOBase::Grid,GOBase::Values,X); 
    }

    template <typename...Args>
    inline auto operator()(Args const&...args) const noexcept{
        return eval(args...);
    }

    /// @brief give view on inner dim grid function
    /// @tparam NewInterpolator optional new interpolator
    /// @tparam N depth of inner grid
    /// @param MI index of inner grid (e.g. inner_slice(i) -> GridFunction(Grid.InnerGrids[i], Values, shifted on Grid.Grid.size() ))
    /// @return 
    template <typename NewInterpolator = Interpolator,typename MultiIndex_t>
    auto inner_slice(const MultiIndex_t & MI){
        return GridFunction<NewInterpolator,decltype(GOBase::Grid.inner(MI)) &,decltype(make_slice(GOBase::Values,0,0))>(
            GOBase::Grid.inner(MI),
            make_slice(
                GOBase::Values,
                GOBase::Grid.LinearPartialIndex(MI),
                GOBase::Grid.inner(MI).size()
            )
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

    INHERIT_DESERIALIZATOR(GOBase,GridFunction)
};

/// @brief make grid function, initializing by Func
/// @param Grid 
/// @param Func object, callable with tuple: Func(tuple(x1,...xn))
template <typename Interpolator =  linear_interpolator,typename GridType,typename LambdaType>
auto make_function_tuple(GridType && Grid,LambdaType && Func){
    typedef typename std::decay<decltype(Func(Grid[Grid.MultiZero()]))>::type value_type;
    std::vector<value_type> values;
    values.reserve(Grid.size());

    for(auto MI = Grid.MultiMultiZero();!Grid.IsEnd(MI);Grid.MultiIncrement(MI)){
        values.push_back(Func(Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,value_type>(std::forward<GridType>(Grid),std::move(values));
}




/// @brief make grid function, initializing by Func
/// @param Grid 
/// @param Func object, callable with pack: Func(x1,...xn)
template <typename Interpolator = linear_interpolator,typename GridType,typename LambdaType>
auto make_function_f(GridType && Grid,LambdaType && Func){
    typedef typename std::decay<decltype(templdefs::apply_tuple(Func,Grid[Grid.MultiZero()]))>::type value_type;
    std::vector<value_type> values;
    values.reserve(Grid.size());

    for(auto MI = Grid.MultiZero();!Grid.IsEnd(MI);Grid.MultiIncrement(MI)){
        //std::cout <<MI << ", " <<  (Grid[MI]) << std::endl; 
        values.push_back(templdefs::apply_tuple(Func,Grid[MI]));
    }
    return GridFunction<Interpolator,typename std::decay<GridType>::type,std::vector<value_type> >
            (std::forward<GridType>(Grid),std::move(values));
}

/// @brief makes GridFunction from grid and vector of values
/// @tparam Interpolator interpolator class with static function interpolate(Grid,values,point)
/// @param Grid 
/// @param Values 
/// @return 
template <typename Interpolator = linear_interpolator,typename GridType,typename VectorType>
auto make_function(GridType && Grid,VectorType && Values){
    return GridFunction<Interpolator,typename std::decay<GridType>::type,
        typename std::decay<VectorType>::type>(std::forward<GridType>(Grid),std::forward<VectorType>(Values));
}

/// @brief makes GridFunction from grid and vector of values
/// @tparam Interpolator interpolator class with static function interpolate(Grid,values,point)
/// @param Grid 
/// @param Values 
/// @return 
template <typename Interpolator = linear_interpolator,typename GridType,typename VectorType>
auto make_function_ref(GridType && Grid,VectorType && Values){
    return GridFunction<Interpolator,GridType,VectorType>
        (std::forward<GridType>(Grid),std::forward<VectorType>(Values));
}


struct default_value_setter{
    template <typename value_t ,typename Values_t>
    inline static void put_value(size_t position,value_t value, Values_t & Values)noexcept(noexcept(Values[position] += value)){
        Values[position] += value;
    }
};

/// @brief Histogramm class
/// @tparam GridType Grid
/// @tparam ContainerType Values 
template <typename GridType,typename ContainerType,typename ValueSetter = default_value_setter>
struct Histogramm:public GridObject<GridType,ContainerType>{
    typedef GridObject<GridType,ContainerType> GOBase;
    using GOBase::GridObject;
    ValueSetter VS;
    inline constexpr Histogramm(GOBase go,ValueSetter VS = {}) noexcept:GOBase(std::move(go)),VS(VS){}

    /// @brief give view on inner dim histo
    /// @tparam NewInterpolator optional new interpolator
    /// @tparam N depth of inner grid
    /// @param MI index of inner grid (e.g. inner_slice(i) -> Histogramm(Grid.InnerGrids[i], Values, shifted on Grid.Grid.size() ))
    /// @return 
    template <typename MultiIndex_t>
    auto inner_slice(const MultiIndex_t & MI){
        return Histogramm<decltype(GOBase::Grid.inner(MI)) &,decltype(make_slice(GOBase::Values,0,0))>(
            this->Grid.inner(MI),
            make_slice(
                GOBase::Values,
                GOBase::Grid.LinearPartialIndex(MI),
                GOBase::Grid.inner(MI).size()
            )
        );
    }


    /// @brief puts value into bin, containing (arg0,...args)
    /// @return true in case of sucsess, false if Grid not contains point 
    template <typename T,typename Arg0,typename...Args>
    inline bool put(T const &value,Arg0 const & arg0,Args const&...args) noexcept{
        return put_point(value,make_point(arg0,args...));
    }

    template <typename T,typename Arg>
    inline bool put(T const &value,Arg const& arg) noexcept{
        static_assert(
            std::tuple_size<std::tuple<Arg>>::value == GOBase::Dim,
            "numbper of args mismatches dimension");
        auto bMI = this->Grid.spos(arg);
        if(!std::get<0>(bMI))
            return false;
        else{
            VS.put_value(
                this->Grid.LinearIndex(std::get<1>(bMI)),
                value,GOBase::Values
            );
            return true;
        }
    }

    template <size_t tuple_shift = 0,typename T,typename Point_t>
    inline bool put_point(T const& value,Point_t const & X)noexcept{
        auto bMI = this->Grid.spos_tuple(X.as_tuple());
        if(!std::get<0>(bMI)){
            return false;
        }
        else{
            VS.put_value(
                this->Grid.LinearIndex(std::get<1>(bMI)),
                value,GOBase::Values
            );
            return true;
        }
    }

    template <typename T, typename...Args>
    inline void put_force_point(T const& value, Point<Args...> const& X) {
        auto MI = this->Grid.template pos_tuple<0>(X.as_tuple());
        VS.put_value(this->Grid.LinearIndex(MI), value, GOBase::Values);
    }
    template <typename T,typename Arg,typename...Args>
    inline void put_force(T const& value, Arg const &arg, Args const&... args) {
        put_force_point(value,make_point(arg,args...));
    }
    template <typename T, typename Arg>
    inline void put_force(T const& value, Arg const& arg) {
        static_assert(
            std::tuple_size<std::tuple<Arg>>::value == GOBase::Dim,
            "numbper of args mismatches dimension");
        auto MI = this->Grid.pos(arg);
        VS.put_value(
            this->Grid.LinearIndex(MI), 
            value, GOBase::Values);

    }

    
    INHERIT_DESERIALIZATOR(GOBase,Histogramm)
};


/// @brief make histogramm
/// @param Grid 
/// @param Values 
/// @return 
template <typename GridType,typename VectorType>
auto make_histo(GridType && Grid,VectorType && Values){
    return Histogramm<typename std::decay<GridType>::type,
        typename std::decay<VectorType>::type>(std::forward<GridType>(Grid),std::forward<VectorType>(Values));
}

/// @brief make histogramm with container vector<value_type>
/// @tparam value_type 
/// @param Grid 
/// @return 
template <typename value_type,typename GridType>
auto make_histo(GridType && Grid){
    std::vector<value_type> values(Grid.size(),0);
    return Histogramm<typename std::decay<GridType>::type,std::vector<value_type>>(std::forward<GridType>(Grid),std::move(values));
}

/// @brief make histogramm, which grid is a referance
/// @param Grid
/// @param Values
/// @return
template <typename GridType,typename VectorType>
auto make_histo_ref(GridType & Grid,VectorType && Values){
    return Histogramm<GridType &,
        typename std::decay<VectorType>::type>(Grid,std::forward<VectorType>(Values));
}

/// @brief make histogramm, which grid and values is a referance
/// @param Grid
/// @param Values
/// @return
template <typename GridType,typename VectorType>
auto make_histo_view(GridType && Grid,VectorType && Values){
    return Histogramm<GridType,VectorType>(
            std::forward<GridType>(Grid),
            std::forward<VectorType>(Values));
}

/// @brief make histogramm, which grid and values are referances
/// @param Grid
/// @param Values
/// @return
template <typename GridType,typename VectorType>
auto make_histo_ref(GridType & Grid,VectorType & Values){
    return Histogramm<GridType &,
        VectorType &>(Grid,Values);
}

/// @brief make histogramm, which grid is a referance, with container vector<value_type>
/// @tparam value_type
/// @param Grid
/// @return
template <typename value_type,typename GridType>
auto make_histo_ref(GridType & Grid){
    std::vector<value_type> values(Grid.size(),0);
    return Histogramm<GridType &,std::vector<value_type>>(Grid,std::move(values));
}



};

#endif
