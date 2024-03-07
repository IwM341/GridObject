#ifndef MULTIDIM_GRID_H
#define MULTIDIM_GRID_H
#include "grid.hpp"
#include "const_container.hpp"
#include "templates.hpp"
#include "object_serialization.hpp"
#include <memory>
#include "iterator_template.hpp"
#include "point.hpp"

namespace grob{
    
    /*!
        \brief Multiindex class for iterating over multidimension grids
        MultiIndex for N = 1 is size_t
    */
    template <typename...IndexType>
    struct MultiIndex;
    
    /// @brief 
    /// @tparam HeadIndex --- type of head index, could be as size_t as MultiIndex<Args...>
    /// @tparam ...TailIndex --- another indexes
    template <typename HeadIndex,typename...TailIndex>
    struct MultiIndex<HeadIndex,TailIndex...>{
        typedef HeadIndex Head;
        typedef MultiIndex<TailIndex...> Tail;
        constexpr static size_t Dim = 1 + Tail::Dim;
        HeadIndex i;
        Tail m;

        inline constexpr MultiIndex(HeadIndex _i,TailIndex..._m) noexcept:i(_i),m(_m...) {}
        
        inline constexpr MultiIndex(HeadIndex _i,MultiIndex<TailIndex...> m) noexcept:i(_i),m(m) {}
        
        template <typename HeadIndex1, typename...TailIndex1>
        inline constexpr MultiIndex(const MultiIndex<HeadIndex1, TailIndex1...> & other):
        i(other.i),m(other.m){
            static_assert(sizeof...(TailIndex1) == sizeof...(TailIndex),
                "error conversion multiindex, should be same dimention"); 
        }

        inline constexpr MultiIndex(){}

        template <size_t index>
        inline constexpr const auto & get(std::integral_constant<size_t,index>) const noexcept{
            return m.get(std::integral_constant<size_t,index-1>{});
        }
        inline constexpr const auto & get(std::integral_constant<size_t,0>) const noexcept{
            return i;
        }

        template <size_t index>
        inline constexpr  auto & get(std::integral_constant<size_t,index>)  noexcept{
            return m.get(std::integral_constant<size_t,index-1>{});
        }
        inline constexpr auto & get(std::integral_constant<size_t,0>)  noexcept{
            return i;
        }

        template <size_t index>
        inline constexpr const auto & get() const noexcept{
            return m.template get(std::integral_constant<size_t,index>{});
        }
        template <size_t index>
        inline constexpr auto & get() noexcept{
            return m.template get(std::integral_constant<size_t,index>{});
        }

        /// @brief 
        constexpr inline bool operator !=(const MultiIndex& mi )const noexcept{
            return (i!=mi.i || m!= mi.m);
        }
        /// @brief 
        constexpr inline bool operator ==(const MultiIndex& mi )const noexcept{
            return (i==m && m== mi.m);
        }
        /// @brief 
        constexpr inline bool operator <(const MultiIndex& mi )const noexcept{
            return (i<mi ? true : (i > mi ? false : m<mi));
        }
        /// @brief 
        constexpr inline bool operator <=(const MultiIndex& mi )const noexcept{
            return (i<mi ? true : (i > mi ? false : m<=mi));
        }
        /// @brief 
        constexpr friend inline bool operator >(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<mi_1;
        }
        /// @brief 
        constexpr friend inline bool operator >=(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<=mi_1;
        }

        /// @brief 
        friend std::ostream & operator <<(std::ostream &os,const MultiIndex & MI){
            os << "(" << MI.i <<", " << MI.m << ")";
            return os;
        }
    };
    template <typename Index>
    struct MultiIndex<Index>{
        constexpr static size_t Dim = 1;
        Index i;
        inline constexpr operator Index & () noexcept{return i;}
        inline constexpr operator Index const& () const noexcept{return i;}

        template <typename...Args>
        inline constexpr MultiIndex<Index>(Args...args) noexcept:i(args...){}
        
        template <size_t index>
        inline constexpr auto & get() noexcept{
            static_assert(index != 0, "MultiIndex get: out of range");
            return i;
        }

        template <size_t index>
        inline constexpr const auto & get() const noexcept{
            static_assert(index != 0, "MultiIndex get: out of range");
            return i;
        }
    };

    template <typename...Args>
    inline constexpr auto make_MI(Args...args) noexcept{
        return MultiIndex<Args...>(args...);
    }
    template <typename Head,typename...TailArgs>
    inline constexpr auto make_MI_rec(Head _head,MultiIndex<TailArgs...> _tail) noexcept{
        return MultiIndex<Head,TailArgs...>(_head,_tail);
    }
    template <typename Head,typename Tail>
    inline constexpr auto make_MI_rec(Head _head,Tail _tail) noexcept{
        return MultiIndex<Head,Tail>(_head,_tail);
    }

    namespace __detail_mi{
        template <typename MultiIndexType,size_t...I>
        inline constexpr auto to_tuple_impl(MultiIndexType const & MI,std::index_sequence<I...>) noexcept{
            return std::make_tuple(MI.template get<I>()...);
        }

        template <typename TupleType,size_t...I>
        inline constexpr auto make_MI_from_tuple_impl(TupleType && MI,std::index_sequence<I...>) noexcept{
            return make_MI(std::get<I>(std::forward<TupleType>(MI))...);
        }

        template <typename Type,size_t...I>
        inline constexpr auto make_MI_from_any_impl(Type && MI,std::true_type is_tuple,std::false_type is_multiindex) noexcept{
            return make_MI_from_tuple_impl(std::forward<Type>(MI),
                std::make_index_sequence<
                    std::tuple_size<typename std::decay<Type>::type>::value
                >{});
        }
        template <typename Type,size_t...I>
        inline constexpr auto make_MI_from_any_impl(Type && MI,std::false_type is_tuple,std::false_type is_multiindex) noexcept{
            return MultiIndex<std::decay_t<Type>>(std::forward<Type>(MI));
        }
        template <typename Type,size_t...I>
        inline constexpr auto make_MI_from_any_impl(Type && MI,std::false_type is_tuple,std::true_type is_multiindex) noexcept{
            return std::decay_t<Type>(std::forward<Type>(MI));
        }

        struct no_type{
            template <typename Tp>
            inline constexpr no_type (Tp &&){}
        };
        template <typename...Args>
        auto is_tuple_impl(std::tuple<Args...>)->std::true_type;
        auto is_tuple_impl(no_type)->std::false_type;

        template <typename T>
        struct is_tuple : public decltype(is_tuple_impl(std::declval<std::decay_t<T>>())){
        };

        template <typename...Args>
        auto is_multiindex_impl(MultiIndex<Args...>)->std::true_type;
        auto is_multiindex_impl(no_type)->std::false_type;

        template <typename T>
        struct is_multiindex : 
        public decltype(is_multiindex_impl(std::declval<std::decay_t<T>>())){
        };
        
    };

    template <typename MultiIndexType>
    inline constexpr auto to_tuple(MultiIndexType const & MI) noexcept {
        return __detail_mi::to_tuple_impl(MI,std::make_index_sequence<std::decay<MultiIndexType>::type::Dim>{});
    }
    template <typename TupleType>
    inline constexpr auto make_MI_from_tuple(TupleType && _Tp) noexcept{
        return __detail_mi::make_MI_from_tuple_impl(
            std::forward<TupleType>(_Tp),
            std::make_index_sequence<std::tuple_size<typename std::decay<TupleType>::type>::value>{}
        );
    }

    template <typename TypeIntMI_t>
    inline constexpr auto make_MI_from_any(TypeIntMI_t && _Tp) noexcept{
        return __detail_mi::make_MI_from_any_impl(
                std::forward<TypeIntMI_t>(_Tp),
                __detail_mi::is_tuple<TypeIntMI_t>{},
                __detail_mi::is_multiindex<TypeIntMI_t>{}
        );
    }


    /// @brief Class, implementing multi dimention quad grid (Dim - dimention)
    /// @tparam GridType type of one dimention grid for 1st index
    /// @tparam GridContainerType type of container, containinng (Dim-1) grids
    template <typename GridType,typename GridContainerType>
    class  MultiGrid{
        protected:

        GridType Grid;
        GridContainerType InnerGrids;
        
        typedef _index_impl::_index_container<
                            typename std::remove_cv<
                                typename std::remove_reference<GridContainerType>::type
                                >::type
                            > _index_type_manager;

        typedef typename _index_type_manager::type GridIndexType;


        
        GridIndexType Indexes;
        //mutable size_t _size;
        const static bool _is_const_container = is_const_container<
        typename std::decay<GridContainerType>::type>::value;
        public:
        

        typedef typename std::decay<GridType>::type::value_type value_type_0;
        typedef typename std::decay<
            typename std::decay<GridContainerType>::type::value_type
        >::type InnerGridType;
        
        typedef typename point_cat<value_type_0,typename InnerGridType::value_type>::type value_type;

        /// @brief dimention
        constexpr static size_t Dim =  1 + InnerGridType::Dim;
 
        /// @brief full size (linear) 
        constexpr size_t size()const{return Indexes.back();}
        
        /// @brief 
        inline MultiGrid()noexcept{}

        friend class mesh_grids_helper;

        /// @brief 
        inline constexpr MultiGrid(GridType Grid, GridContainerType InnerGrids)
        noexcept(_index_type_manager::is_noexcept):
        Grid(std::forward<GridType>(Grid)),
        InnerGrids(std::forward<GridContainerType>(InnerGrids)){
            Indexes = _index_type_manager::Create(this->InnerGrids);
        }

        /// @brief MultiIndex of zeros
        inline auto MultiZero() const{return make_MI_rec(Grid.MultiZero(),InnerGrids[0].MultiZero());}

        typedef decltype(make_MI_rec(Grid.MultiZero(), InnerGrids[0].MultiZero())) MultiIndexType;

        /// @brief cheks if MultiIndex out of range
        /// @param mi 
        /// @return 
        template <typename MultiIndexType_in = MultiIndexType>
        inline constexpr size_t IsEnd(MultiIndexType_in const& mi)const noexcept{
            return Grid.IsEnd(mi.i);
        }

        /// @brief increments MultiIndex in order to loop thorough grid
        /// @param mi 
        template <typename MultiIndexType_in = MultiIndexType>
        inline void MultiIncrement(MultiIndexType_in & mi) const noexcept{
            size_t i = Grid.LinearIndex(mi.i);
            InnerGrids[i].MultiIncrement(mi.m);
            if(InnerGrids[i].IsEnd(mi.m)){
                Grid.MultiIncrement(mi.i);
                mi.m = InnerGrids[i].MultiZero();
            }
        }

        /// @brief gives linear position of Multiindex(i0,indexes...)
        //template <typename...TailIndex>
        inline size_t LinearIndex(
            //MultiIndex<typename MultiIndexType::Head,TailIndex...> 
            MultiIndexType const & mi)const noexcept
        {
            size_t i = Grid.LinearIndex(mi.i);
            return Indexes[i] + InnerGrids[i].LinearIndex(mi.m);
        }

        ///@brief gives linear position of Multiindex(i0,indexes...)
        template <typename HeadIndex>
        inline size_t LinearPartialIndex(HeadIndex const & mi)const noexcept
        {
            return Indexes[Grid.LinearPartialIndex(mi)];
        }

        /// @brief return MultiIndex  corresponding to linear index (works with time O(log2(N)^Dim) )
        inline MultiIndexType FromLinear (size_t i)const noexcept{
            size_t index_ = _index_impl::find_index(Indexes,i);
            return {Grid.FromLinear(index_),
                        InnerGrids[index_].FromLinear(i-Indexes[index_])
                    };
        }


        /// @brief check if x1..xn into grid
        template <typename Arg, typename...Args>
        inline bool contains(Arg const& arg,Args const&...args)const noexcept{
            return Grid.contains(arg) && 
            InnerGrids[ Grid.LinearIndex(Grid.pos(arg))].contains(args...);
        }
        /*
        /// @brief check if x1..xn into grid
        template <typename...Args,
                    typename = 
                        std::enable_if<sizeof...Args > value_type::Dim,bool>::type
                >
        inline bool contains(Args const&...args)const noexcept{
            return contains_tuple(value_type(args...).as_tuple());
        }*/

        /// @brief same as contains, (x1,...xn) packed into tuple
        template <size_t tuple_index = 0,typename...Args>
        inline bool contains_tuple(std::tuple<Args...> const& T) const noexcept{
            return Grid.contains(std::get<tuple_index>(T)) &&
            InnerGrids[ Grid.LinearIndex(Grid.pos(std::get<tuple_index>(T)))].template 
                contains_tuple<tuple_index+1>(T);
        }

        /// @brief check and find MultiIndex matching tuple(args...) 
        /// @return tuple(bool: is_contains,MultiIndex  pos)
        template <typename...Args>
        inline constexpr auto spos(Args &&...args) const noexcept{
            return spos_tuple(make_point(std::forward<Args>(args)...).as_tuple());
        }

        /// @brief check and MultiIndex matching tuple(args...) 
        /// @return tuple(bool: is_contains,Multiindex  pos)
        template <size_t tuple_index = 0,typename...Args>
        inline constexpr auto spos_tuple(std::tuple<Args...> const& TX) const noexcept{
            bool b = true;
            MultiIndexType MI;
            fill_index_tuple_save_impl(TX,MI,b);
            return std::make_tuple(b,MI);
        }
        template <size_t tuple_index = 0,typename Tuple,typename Index>
        inline constexpr void fill_index_tuple_save_impl(Tuple const & _Tp,
                                                    Index & index_to_fill,
                                                    bool & b)const noexcept{
            Grid.template fill_index_tuple_save_impl<tuple_index>(_Tp,index_to_fill.i,b);
            if(b)
                InnerGrids[Grid.LinearIndex(index_to_fill.i)].template 
                fill_index_tuple_save_impl<tuple_index+1>(_Tp,index_to_fill.m,b);
        }

        /// @brief MultiIndex matching args... 
        template <typename T,typename...Other>
        inline auto pos(T const & x,Other const&...Y) const noexcept{
            auto i =  Grid.pos(x);
            return MultiIndexType(i,InnerGrids[Grid.LinearIndex(i)].pos(Y...));
        }
        //template <typename T>
        //inline  auto pos(T const & x) const noexcept{
        //    return Grid.pos(x);
        //}

        /// @brief MultiIndex matching tuple(args...) 
        template <size_t tuple_index,typename...T>
        inline  auto pos_tuple(std::tuple<T...> const & X) const noexcept{
            static_assert(sizeof...(T) != Dim,"in pos_tuple tuple size doesn't match Dim");
            auto i = Grid.template pos_tuple<tuple_index>(X);
            return MultiIndexType(i,
                InnerGrids[Grid.LinearInde(i)].template pos_tuple<tuple_index+1>(X)
            );
        }       

        /// @brief gives multidim point or rectangle 
        /// @return 
        //template <typename IndexHead,typename...IndexTail> 
        inline auto  operator [] (
                const MultiIndexType/*MultiIndex<IndexHead, IndexTail...>*/& mi
            )const noexcept {
            return make_point_ht(Grid[mi.i],InnerGrids[Grid.LinearIndex(mi.i)][mi.m]);
        } 
        
        //template <typename IndexHead> 
        //inline auto  operator [] (const MultiIndex<IndexHead> & i)const noexcept {
        //    return Grid[i.i];
        //}
        

        /// @brief grid of first dim 
        inline auto const & grid()const noexcept {
            return Grid;
        }
        /// @brief gives inner grid 
        inline auto const & inner()const noexcept {
            return InnerGrids;
        }

        /// @brief gives inner grid indexed by MultiIndex
        /// @tparam N 
        /// @param mi 
        /// @return inner(int i) -> InnerGrids[i], inner( (i,j)) -> InnerGrids[i].inner(j) ...
        template <typename IndexHead,typename...IndexTail> 
        inline auto const & inner(const MultiIndex<IndexHead,IndexTail...> &mi)const noexcept {
            return InnerGrids[Grid.LinearIndex(mi.i)].inner(mi.m);
        }

        template <typename IndexType> 
        inline auto const & inner(const  IndexType &i)const noexcept {
            return InnerGrids[Grid.LinearIndex(i)];
        }

        struct iterator{
            protected:
            MultiGrid const&  __MG;
            MultiIndexType Position;

            public:
            inline const auto & index() const{
                return Position;
            }
            inline auto & index(){
                return Position;
            }
            inline operator MultiGrid const& ()const{
                return __MG;
            }

            iterator(MultiGrid const&  __MG, MultiIndexType Position):__MG(__MG),Position(Position){}
            inline iterator & operator ++(){
                __MG.MultiIncrement(Position);
                return *this;
            }
            inline iterator & operator ++(int){
                iterator __tmp = *this;
                ++(*this);
                return __tmp;
            }
            inline auto operator *() const{
                return __MG[Position];
            }

            template <typename T>
            inline bool operator !=(T const &)const{
                return !__MG.IsEnd(Position);
            }
        };
        inline iterator begin() const{
            return iterator{*this,MultiZero()};
        }
        inline iterator end() const{
            return iterator{*this,MultiZero()};
        }
        inline iterator cbegin()const{
            return begin();
        }
        inline iterator cend()const{
            return end();
        }
        
        /// @brief  
        friend std::ostream & operator << (std::ostream & os,const MultiGrid &MG){
            std::ostringstream S;
            S << "MultiGrid(" << MG.Grid <<", ";
            //MG.InnerGrids = 0;
            stools::print_vector::__print__including__vectors__(S , MG.InnerGrids);
            S << ")";
            return os << S.str();
        }
        
        
        SERIALIZATOR_FUNCTION(PROPERTY_NAMES("Grid","InnerGrids"),
                              PROPERTIES(Grid,InnerGrids))
        WRITE_FUNCTION(Grid,InnerGrids)
        DESERIALIZATOR_FUNCTION(MultiGrid,
            PROPERTY_NAMES("Grid","InnerGrids"),
            PROPERTY_TYPES(Grid,InnerGrids))
        READ_FUNCTION(MultiGrid,PROPERTY_TYPES(Grid,InnerGrids))

    };

    namespace gtypes{
        template <typename GridType,typename InnerGrid,typename...Alloc>
        struct _inner_container_type{
            typedef std::vector<InnerGrid,Alloc...> type;

            template <typename Initializer>
            static type Create(const GridType & Grid,Initializer && Lmbd){
                type CC;
                CC.reserve(Grid.size());
                for(size_t i=0;i<Grid.size();++i){
                    CC.push_back(Lmbd(i));
                }
                return CC;
            };
        };
        
        template <typename T,size_t size,typename InnerGrid>
        struct _inner_container_type<GridArray<T,size>,InnerGrid>{
            typedef std::array<InnerGrid,size> type;
            template <typename GridType,typename Initializer>
            static type Create(const GridType & Grid,Initializer && Lmbd) noexcept{
                type CC;
                for(size_t i=0;i<CC.size();++i){
                    CC[i] = Lmbd(i);
                }
                return CC;
            };
        };
        
        template <typename GridTypeA, typename GridContainerType>
        struct _build_helper{
            typedef typename std::decay<GridTypeA>::type GType;
            typedef typename std::decay<GridContainerType>::type GCType;
            typedef MultiGrid<GType,GCType> MGType;
        };
    }

    /// @brief makes grid
    /// @param GridA first dim grid
    /// @param GContainer ontainer of  inner grids
    /// @return MultiGrid(GridA,GContainer)
    template <typename GridTypeA, typename GridContainerType>
    constexpr inline auto make_grid(GridTypeA && GridA,GridContainerType && GContainer){
        return typename gtypes::_build_helper<GridTypeA,GridContainerType>::MGType
            (std::forward<GridTypeA>(GridA),std::forward<GridContainerType>(GContainer));
    }

    /// @brief makes grid from funcion initializer 
    /// @param GridA grid1
    /// @param LambdaInitializer callable instance, LambdaInitializer(int i) -> i'th inner grid
    /// @return 
    template <typename GridTypeA, typename LambdaInitializerType>
    inline auto make_grid_f(GridTypeA && GridA,LambdaInitializerType && LambdaInitializer){
        return make_grid(
            std::forward<GridTypeA>(GridA),
            gtypes::_inner_container_type<
                    typename std::decay<GridTypeA>::type,
                    decltype(LambdaInitializer(std::declval<size_t>()))
                >::Create(GridA,LambdaInitializer)
        );
    }

    /// @brief meshes grids
    /// @param GridA 
    /// @param GridB 
    /// @return decart product of two grids
    template <typename GridTypeA,typename GridTypeB>
    inline auto mesh_grids(GridTypeA && GridA,GridTypeB && GridB){
            size_t size = GridA.size();
            return make_grid(std::forward<GridTypeA>(GridA),
                ConstValueVector<typename std::decay<GridTypeB>::type>(std::forward<GridTypeB>(GridB),size));   
    }

    
};



#endif//MULTIDIM_GRID_H
