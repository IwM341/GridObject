#ifndef MULTIDIM_GRID_H
#define MULTIDIM_GRID_H
#include "grid.hpp"
#include "const_container.hpp"
#include "templates.hpp"
#include <memory>



namespace grob{
    
    /*!
        \brief Multiindex class for iterating over multidimension grids
        MultiIndex for N = 1 is size_t
    */
    template <size_t N>
    struct MultiIndex{
        size_t i;
        MultiIndex<N-1> m;
        constexpr static size_t Dim = N;
        
        
        template <typename...Args>
        constexpr inline MultiIndex(size_t i = 0,Args...args) noexcept:i(i),m(args...){}
        
        template <size_t M>
        constexpr inline MultiIndex(size_t i,MultiIndex<M-1> const& m) noexcept:i(i),m(m){}

        template <size_t tuple_start,typename TupleType>
        inline constexpr void from_tuple(TupleType const & IT)noexcept{
            i = std::get<tuple_start>(IT);
            m.template from_tuple<tuple_start+1>(IT);
        }

        template <typename...TupleTypes>
        constexpr inline MultiIndex(std::tuple<TupleTypes...> const & IT){
            from_tuple<0,std::tuple<TupleTypes...>>(IT);
        }


        constexpr inline MultiIndex& operator = (size_t j)noexcept{
            i=0;
            m = j;
            return *this;
        }
        constexpr inline bool operator !=(const MultiIndex& mi )const noexcept{
            return (i!=mi.i || m!= mi.m);
        }
        constexpr inline bool operator ==(const MultiIndex& mi )const noexcept{
            return (i==m && m== mi.m);
        }
        constexpr inline bool operator <(const MultiIndex& mi )const noexcept{
            return (i<mi ? true : (i > mi ? false : m<mi));
        }
        constexpr inline bool operator <=(const MultiIndex& mi )const noexcept{
            return (i<mi ? true : (i > mi ? false : m<=mi));
        }
        constexpr friend inline bool operator >(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<mi_1;
        }
        constexpr friend inline bool operator >=(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<=mi_1;
        }

        inline std::string toString() const noexcept{
            return std::to_string(i) + ", " + m.toString();
        }
        friend std::ostream & operator <<(std::ostream &os,const MultiIndex & MI){
            os << ("MultiIndex(" + MI.toString() + ")");
            return os;
        }
    };

    template <>
    struct MultiIndex<1>{
        size_t i;
        constexpr static size_t Dim = 1;

        template <size_t tuple_start,typename TupleType>
        inline constexpr void from_tuple(TupleType const & IT)noexcept{
            i = std::get<tuple_start>(IT);
        }

        template <typename...TupleTypes>
        constexpr inline MultiIndex(std::tuple<TupleTypes...> const & IT):MultiIndex(from_tuple<0,std::tuple<TupleTypes...>>(IT)){}

        constexpr inline MultiIndex<1>(size_t i=0) noexcept:i(i){}
        constexpr inline operator size_t ()const noexcept{return i;}
        constexpr inline operator size_t &() noexcept{return i;}

        inline std::string toString() const noexcept{
            return std::to_string(i);
        }
    };

    

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
        
        mutable GridIndexType Indexes;
        //mutable size_t _size;
        const static bool _is_const_container = is_const_container<
        typename std::decay<GridContainerType>::type>::value;
        public:
        

        typedef typename std::decay<GridType>::type::value_type value_type;
        typedef typename std::decay<GridContainerType>::type::value_type InnerGridType;
        constexpr static size_t Dim =  1 + InnerGridType::Dim;
        
        typedef typename std::conditional<_is_const_container,
                RectConst<value_type,typename InnerGridType::RectType>,
                RectCommon<value_type,typename InnerGridType::RectType>
            >::type RectType;
        typedef typename std::conditional<_is_const_container,
                    RectConst<size_t,typename InnerGridType::IndexRectType>,
                    RectCommon<size_t,typename InnerGridType::IndexRectType>
                >::type IndexRectType;

        typedef typename templdefs::tuple_cat<value_type,typename InnerGridType::PointType>::type PointType;


        constexpr size_t size()const{return Indexes.back();}
        
        inline MultiGrid()noexcept{
            static_assert(is_histo_grid<typename std::decay<GridType>::type>::value == 
            is_histo_grid<InnerGridType>::value,"Grid types mismatches at MultiGrid");
        }

        template <bool is_grid_1>
        friend class mesh_grids_helper;

        inline constexpr MultiGrid(GridType Grid, GridContainerType InnerGrids)
        noexcept(_index_type_manager::is_noexcept):
        Grid(std::forward<GridType>(Grid)),
        InnerGrids(std::forward<GridContainerType>(InnerGrids)){
            static_assert(is_histo_grid<typename std::decay<GridType>::type>::value == 
            is_histo_grid<InnerGridType>::value,"Grid types mismatches at MultiGrid");
            Indexes = _index_type_manager::Create(this->InnerGrids);
        }

        inline static MultiIndex<Dim> Multi0(){return MultiIndex<Dim>();}
        inline MultiIndex<Dim> MultiSize()const{return MultiIndex<Dim>(InnerGrids.size(),MultiIndex<Dim-1>());}

        template <typename...Args>
        inline bool contains(Args const&...args)const noexcept{
            return contains_tuple(std::make_tuple(args...));
        }

        template <size_t tuple_index = 0,typename...Args>
        inline bool contains_tuple(std::tuple<Args...> const& T) const noexcept{
            if(!Grid.contains(std::get<tuple_index>(T)))
                return false;
            size_t i = Grid.pos(std::get<tuple_index>(T));
            if(_is_const_container){
                return InnerGrids[i].template contains_tuple<tuple_index+1>(T);
            }
            else{
                return InnerGrids[i].template contains_tuple<tuple_index+1>(T) && InnerGrids[i+1].template contains<tuple_index+1>(T);
            }
        }

        template <typename...Args>
        auto spos(Args const&...args){
            return spos(std::make_tuple(args...));
        }

        template <typename...Args>
        auto spos(std::tuple<Args...> const& TX){
            bool b = true;
            MultiIndex<Dim> MI;
            fill_index_tuple_save_impl(TX,b,MI);
            return std::make_tuple(b,MI);
        }

        template <typename T,typename...Other>
        inline auto pos(T const & x,Other const&...Y) const noexcept{
            size_t i =  Grid.pos(x);
            return MultiIndex<std::tuple_size<std::tuple<T,Other...>>::value>(i,InnerGrids[i].pos(Y...));
        }
        template <typename T>
        inline  auto pos(T const & x) const noexcept{
            return Grid.pos(x);
        }
        template <typename...T>
        inline  auto pos(std::tuple<T...> const & X) const noexcept{
            MultiIndex<std::tuple_size<std::tuple<T...>>::value> MI;
            fill_index_tuple_impl<0>(X,MI);
            return MI;
        }

        template <typename T,typename...Other>
        inline auto FindIndex(T const&x,Other const&...args) const noexcept{
            return FindIndex(std::make_tuple(x,args...));
        }

        template <typename T,typename...Args>
        inline auto FindIndex(std::tuple<T,Args...> const & X) const noexcept{
            typedef decltype(InnerGrids[std::declval<size_t>()].FindIndex(std::declval<Args>()...)) InnerIndexType;
            typedef typename std::conditional<_is_const_container,
                            RectConst<size_t,InnerIndexType>,
                            RectCommon<size_t,InnerIndexType>>::type RetIndexType;
            RetIndexType I;
            fill_rect_index_impl<0>(X,I);
            return I;
        }
        template <typename T>
        inline auto FindIndex(std::tuple<T> const & X) const noexcept{
            return Grid.FindIndex(std::get<0>(X));
        }


        template <typename T,typename...Other>
        inline auto FindElement(T const& x,Other const&...args) const noexcept{
            return FindElement(std::make_tuple(x,args...));
        }

        template <typename...Args>
        inline auto FindElement(std::tuple<Args...> const & X) const noexcept{
            std::tuple<IndexRectType, RectType> IR;
            fill_element<0>(X,std::get<0>(IR),std::get<1>(IR));
            return IR;
        }

        inline constexpr size_t IsEnd(MultiIndex<Dim> const& mi)const noexcept{
            return mi.i == Grid.size();
        }
        inline void MultiIncrement(MultiIndex<Dim> & mi) const noexcept{
            InnerGrids[mi.i].MultiIncrement(mi.m);
            if(InnerGrids[mi.i].IsEnd(mi.m)){
                ++mi.i;
                mi.m = 0;
            }
        }


        template <typename...Ints>
        inline size_t LinearIndex(size_t i0,Ints...indexes)const noexcept
        {
            return Indexes[i0] + InnerGrids[i0].LinearIndex(indexes...);
        }
        inline size_t LinearIndex(size_t i)const noexcept
        {
            return Indexes[i];
        }
        template <size_t N>
        inline size_t LinearIndex(const MultiIndex<N> &mi)const noexcept{
            return Indexes[mi.i] + InnerGrids[mi.i].LinearIndex(mi.m);
        }

        inline size_t LinearIndex(const MultiIndex<1> &i)const noexcept{
            return Indexes[i];
        }

        inline MultiIndex<Dim> FromLinear (size_t i)const noexcept{
            size_t index_ = find_index_sorted_with_guess(Indexes,i) ;
            size_t prev_size_ = Indexes[index_];
            return MultiIndex<Dim>(index_,InnerGrids[index_].FromLinear(i-prev_size_));
        }



        inline auto const & first()const noexcept {
            return Grid;
        }
        inline auto const & inner()const noexcept {
            return InnerGrids;
        }

        template <size_t N> 
        inline auto const & inner(const  MultiIndex<N> &mi)const noexcept {
            return InnerGrids[mi.i].inner(mi.m);
        }
        //template <> 
        inline auto const & inner(const  MultiIndex<1> &i)const noexcept {
            return InnerGrids[i];
        }

        template <size_t N,typename std::enable_if<N!=1,bool>::type = true> 
        inline auto  operator [] (const  MultiIndex<N> &mi)const noexcept {
            typename templdefs::tuple_cat<value_type,typename std::decay<decltype(InnerGrids[std::declval<size_t>()][mi.m])>::type>::type P;
            //typename std::decay<decltype(InnerGrids[mi.m])>::type _DD_;

            
            //_DD_ = 1;

            fill_tuple<0>(P,mi);
            return P;
        }
        inline auto  operator [] (const  MultiIndex<1> &i)const noexcept {
            return Grid[i];
        }
        

        template <size_t shift,typename RectIndex,typename...T>
        inline  void fill_rect_index_impl(std::tuple<T...> const & TX,RectIndex & MI) const noexcept{
            Grid.template fill_rect_index_impl<shift>(TX,MI.first());
            InnerGrids[MI.left()].template fill_rect_index_impl<shift+1>(TX,MI.inner_left());
            if(!_is_const_container){
                InnerGrids[MI.right()].template fill_rect_index_impl<shift+1>(TX,MI.inner_right());
            }
        }

        template <size_t shift = 0,typename...T>
        inline  void fill_index_tuple_save_impl(std::tuple<T...> const & TX,bool & b,MultiIndex<Dim> & MI) const noexcept{
            if(b){
                Grid.template fill_index_tuple_save_impl<shift>(TX,b,MI.i);
                if(b){
                    InnerGrids[MI.i].template fill_index_tuple_save_impl<shift+1>(TX,b,MI.m);
                }
            }
        }

        template <size_t shift,size_t N ,typename...T>
        inline  void fill_index_tuple_impl(std::tuple<T...> const & TX,MultiIndex<N> & MI) const noexcept{
            Grid.template find_index_tuple_impl<shift>(TX,MI.i);
            InnerGrids[MI.i].template fill_index_tuple_impl<shift+1>(TX,MI.m);
        }
        template <size_t shift,typename...T>
        inline  void fill_index_tuple_impl(std::tuple<T...> const & TX,size_t & MI) const noexcept{
            Grid.template find_index_tuple_impl<shift>(TX,MI);
        }

        template <size_t shift,size_t N,typename...T>
        inline  void fill_tuple(std::tuple<T...> & TX,MultiIndex<N> const& MI) const noexcept{
            Grid.template fill_tuple<shift>(TX,MI.i);
            InnerGrids[MI.i].template fill_tuple<shift+1>(TX,MI.m);
        }
        template <size_t shift,typename...T>
        inline  void fill_tuple(std::tuple<T...> & TX,MultiIndex<1> const& MI) const noexcept{
            Grid.template fill_tuple<shift>(TX,MI);
        }

        template <size_t shift,typename IndexElement,typename RectElement,typename...T>
        inline  void fill_element(std::tuple<T...> const& TX,IndexElement & IE,RectElement & RE) const noexcept{
            Grid.template fill_element<shift>(TX,IE.first(),RE.first());
            if(_is_const_container){
                InnerGrids[IE.left()].template fill_element<shift+1>(TX,IE.inner_left(),RE.inner_left());
            }else{
                InnerGrids[IE.left()].template fill_element<shift+1>(TX,IE.inner_left(),RE.inner_left());
                InnerGrids[IE.right()].template fill_element<shift+1>(TX,IE.inner_right(),RE.inner_right());
            }
        }

        friend std::ostream & operator << (std::ostream & os,const MultiGrid &MG){
            using namespace stools::print_vector;
            std::ostringstream S;
            S << "MultiGrid(" << MG.Grid <<", ";
            S << MG.InnerGrids;
            S << ")";
            return os << S.str();
        }
        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(2,
                [](size_t i){ return (!i?"Grid":"InnerGrids");},
                [this,&S](size_t i){
                    return ( !i? stools::Serialize(Grid,S) : 
                        stools::Serialize(InnerGrids,S));
                    }
            );
        }

        template <typename Object,typename DeSerializer>
        static MultiGrid DeSerialize(Object && Obj,DeSerializer && DS){
            return MultiGrid(stools::DeSerialize<GridType>(DS.GetProperty(Obj,"Grid"),DS),
                            stools::DeSerialize<GridContainerType>(DS.GetProperty(Obj,"InnerGrids"),DS));
        }

        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            stools::init_serialize(Grid,S.GetProperty(Object,"Grid"));
            stools::init_serialize(InnerGrids,S.GetProperty(Object,"InnerGrids"));
        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w)const{
            stools::write(Grid,w);
            stools::write(InnerGrids,w);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            stools::init_read(Grid,r);
            stools::init_read(Grid,r);
        }
        template <typename ReaderStreamType>
        static auto read(ReaderStreamType && r){
            auto Grid = stools::read<typename std::decay<GridType>::type>(r);
            auto InnerGrids = stools::read<typename std::decay<GridContainerType>::type>(r);
            return MultiGrid(std::move(Grid),std::move(InnerGrids));
        }


    };

    template <typename GridType,typename GridContainerType>
    struct MultiGridHisto: public MultiGrid<GridType,GridContainerType>{
        typedef MultiGrid<GridType,GridContainerType> GBase;
        using GBase::GBase;
        typedef RectConst<typename GridType::value_type,typename GBase::InnerGridType::ElementType> ElementType;
        
        MultiGridHisto(GBase Base):GBase(std::move(Base)){}

        template <typename T,typename...Other>
        inline auto FindElement(T const& x,Other const&...args) const noexcept{
            return FindElement(std::make_tuple(x,args...));
        }

        template <typename...Args>
        inline auto FindElement(std::tuple<Args...> const & X) const noexcept{
            typedef MultiIndex<std::tuple_size<std::tuple<Args...>>::value> Index;
            std::tuple<Index, decltype((*this)[std::declval<Index>()])> IR;
            fill_element<0>(X,std::get<0>(IR),std::get<1>(IR));
            return IR;
        }
        
        template <size_t shift,size_t N,typename RectElement,typename...T>
        inline constexpr void fill_element(std::tuple<T...> & TX,MultiIndex<N> & MI,RectElement & RE) const noexcept{
            GBase::Grid.template fill_element<shift>(TX,MI.i,RE.first());
            GBase::InnerGrids[MI.i].template fill_element<shift+1>(TX,MI.m,RE.inner());
        }

        template <typename RectElement,size_t N>
        inline void fill_rect(RectElement & R,MultiIndex<N> const& MI){
            GBase::Grid.fill_rect(R.first(),MI.i,R.first());
            GBase::InnerGrids[MI.i].fill_rect(R.inner(),MI.m);
        }

        template <size_t N> 
        inline auto  operator [] (const  MultiIndex<N> &mi)const noexcept {
            return(make_rect(GBase::Grid[mi.i],GBase::InnerGrids[mi.i][mi.m]));
        }
        inline auto  operator [] (const  size_t &mi)const noexcept {
            return GBase::Grid[mi];
        }

    };
    template <typename GridType,typename GridContainerType>
    struct is_histo_grid<MultiGridHisto<GridType,GridContainerType>>{
        constexpr static bool value = true;
    };

    namespace gtypes{
        template <typename GridType,typename InnerGrid>
        struct _inner_container_type{
            typedef std::vector<InnerGrid> type;

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
        
        template <typename T,size_t size,typename InnerGrid>
        struct _inner_container_type<GridArrayHisto<T,size>,InnerGrid>:
            public _inner_container_type<GridArray<T,size>,InnerGrid>{
            using _inner_container_type<GridArray<T,size>,InnerGrid>::_inner_container_type;
        };

        template <typename GridTypeA, typename GridContainerType>
        struct _build_helper{
            typedef typename std::decay<GridTypeA>::type GType;
            typedef typename std::decay<GridContainerType>::type GCType;
            typedef MultiGrid<GType,GCType> MGType;
        };
        template <typename GridTypeA, typename GridContainerType>
        struct _build_helper_histo:public _build_helper<GridTypeA,GridContainerType>{
            typedef _build_helper<GridTypeA,GridContainerType> BBase;
            using BBase::BBase;
            typedef MultiGridHisto<typename BBase::GType,typename BBase::GCType> MGType;
        };
    }

    template <typename GridTypeA, typename GridContainerType>
    constexpr inline auto make_grid_point(GridTypeA && GridA,GridContainerType && GContainer){
        return typename gtypes::_build_helper<GridTypeA,GridContainerType>::MGType
            (std::forward<GridTypeA>(GridA),std::forward<GridContainerType>(GContainer));
    }
    template <typename GridTypeA, typename GridContainerType>
    constexpr inline auto make_grid_histo(GridTypeA && GridA,GridContainerType && GContainer){
        return typename gtypes::_build_helper_histo<GridTypeA,GridContainerType>::MGType
            (HistoCast(GridA),std::forward<GridContainerType>(GContainer));
    }

    template <typename GridTypeA, typename GridContainerType>
    constexpr inline typename std::conditional<
                                    is_histo_grid<
                                        typename std::decay<GridTypeA>::type
                                    >::value,typename gtypes::_build_helper_histo<GridTypeA,GridContainerType>::MGType,
                                    typename gtypes::_build_helper<GridTypeA,GridContainerType>::MGType
                                >::type 
        make_grid(GridTypeA && GridA,GridContainerType && GContainer){
        return make_grid_point(std::forward<GridTypeA>(GridA),std::forward<GridContainerType>(GContainer));
    }

    template <typename GridTypeA, typename LambdaInitializerType>
    auto make_grid_f(GridTypeA && GridA,LambdaInitializerType && LambdaInitializer){
        return make_grid(
            std::forward<GridTypeA>(GridA),
            gtypes::_inner_container_type<
                    typename std::decay<GridTypeA>::type,
                    typename std::invoke_result<LambdaInitializerType,size_t>::type
                >::Create(GridA,LambdaInitializer)
        );
    }


    template <typename GridTypeA,typename GridTypeB>
    auto mesh_grids(GridTypeA && GridA,GridTypeB && GridB);

    template <bool is_1dim_grid>
    struct mesh_grids_helper{
        template <typename GridTypeA,typename GridTypeB>
        static auto mesh_grids_(GridTypeA && GridA,std::shared_ptr<GridTypeB> GridB){
            size_t size = GridA.size();
            return make_grid(std::forward<GridTypeA>(GridA),ConstSharedValueVector<GridTypeB>
                (size,GridB));
        }
    };
    template <>
    struct mesh_grids_helper<false>{
        template <typename GridTypeA,typename GridTypeB>
        static auto mesh_grids_(GridTypeA && GridA,std::shared_ptr<GridTypeB> GridB){
            size_t size = GridA.Grid.size();
            return make_grid(GridA.Grid,map(GridA.InnerGrids,[&GridB](auto grid){
                return mesh_grids_helper<is_grid1<typename std::decay<decltype(grid)>::type>::value>::mesh_grids_(grid,GridB);
            }));
        }
    };

    template <typename GridTypeA,typename GridTypeB>
    auto mesh_grids(GridTypeA && GridA,GridTypeB && GridB){
        return mesh_grids_helper<is_grid1<typename std::decay<GridTypeA>::type>::value>
        ::mesh_grids_(std::forward<GridTypeA>(GridA),
            std::make_shared<typename std::decay<GridTypeB>::type>(std::forward<GridTypeB>(GridB)));
    }

    
};



#endif//MULTIDIM_GRID_H
