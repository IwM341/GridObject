#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <array>
#include <sstream>
#include "rectangle.hpp"
#include "serialization.hpp"
#include "templates.hpp"
#include "container_shift.hpp"
#include "iterator_template.hpp"
#include "object_serialization.hpp"

/*!
    \brief implementation and definition of grid
*/
namespace grob{


    /*
    template <typename T>
    struct virtual_container{
        typedef T value_type;
        virtual T operator [](size_t index)const = 0;
        virtual size_t size() const = 0;
        typedef _const_iterator_template<virtual_container,T> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,_size);}
        virtual ~virtual_container(){}
    };

    template <typename Container>
    struct cnt_virtualize: public virtual_container<typename std::decay<std::declval<Container>()[0]>::type > {
        Container _body;
        cnt_virtualize(Container const & _body)noexcept:_body(_body){}
        cnt_virtualize(Container && _body)noexcept:_body(std::move(_body)){}
        T operator [](size_t index)const noexcept{
            return _body[index];
        }
        size_t size(){
            return _body.size();
        }
    };

    template <typename T>
    struct virtual_container_wrapper{
        std::shared_ptr<virtual_container> _body;
        inline T operator [](size_t index)const {
            return (*_body)[index];
        }
        inline size_t size(){

        }
    };
    */
   
    struct vector_array_grid_helper{
        template <typename cnt_type,typename T>
        constexpr inline static  size_t pos_impl(cnt_type const & Grd,T const & x) noexcept{
            return __find_index_sorted_with_guess(Grd,x);
        }
        template <typename cnt_type,typename T>
        constexpr inline static bool contain_impl(cnt_type const & Grd,T const & x) noexcept{
            return Grd.front() <= x && x <= Grd.back();
        }
    };
    struct vector_array_grid_helper_nearest{
        template <typename cnt_type,typename T>
        constexpr inline static  size_t pos_impl(cnt_type const & Grd,T const & x) noexcept{
            size_t i =  __find_index_sorted_with_guess(Grd,x);
            if(i < 2 || x - Grd[i] < Grd[i+1] - x){
                return i;
            } else {
                return i + 1;
            }
        }
        template <typename cnt_type,typename T>
        constexpr inline static bool contain_impl(cnt_type const & Grd,T const & x) noexcept{
            return vector_array_grid_helper::contain_impl(Grd,x);
        }
    };

    template <typename Container>
    void __print_container__(std::ostream & os,Container const& cnt);


    template <typename...Args>
    void  __print_container__(std::ostream & os,const std::vector<Args...> & VG){
        std::ostringstream internal_stream;
        internal_stream << "vector(" << VG.size() << ")[";
        if(VG.size()){
            internal_stream << VG[0];
        }
        for(size_t i=1;i<VG.size();++i){
            internal_stream << ", " <<VG[i];
        }
        internal_stream << "]";
        os << internal_stream.str();
    }
    template <typename T,size_t N>
    void  __print_container__(std::ostream & os,const std::array<T,N> & VG){
        std::ostringstream internal_stream;
        internal_stream << "array(" << VG.size() << ")[";
        if(VG.size()){
            internal_stream << VG[0];
        }
        for(size_t i=1;i<VG.size();++i){
            internal_stream << ", " <<VG[i];
        }
        internal_stream << "]";
        os << internal_stream.str();
    }

    template <typename Container>
    struct __default_grid_constructor__{
        constexpr static bool constructable = false;
    };

    template <typename T,typename...Args>
    struct __default_grid_constructor__<std::vector<T,Args...>>{
        constexpr static bool constructable = true;
        static inline std::vector<T,Args...> construct(T const &a,T const & b,size_t size) noexcept{
            std::vector<T,Args...> ret;
            ret.reserve(size);
            for(size_t i=0;i<size;++i){
                ret.push_back( a + i*(b-a)/(size-1));
            }
            return ret;
        }
    };

    template <typename T,size_t _size>
    struct __default_grid_constructor__<std::array<T,_size>>{
        constexpr static bool constructable = true;
        static inline std::array<T,_size> construct(T const &a,T const & b,size_t size) noexcept{
            std::array<T,_size> ret;
            for(size_t i=0;i<_size;++i){
                ret[i] = ( a + i*(b-a)/(_size-1));
            }
            return ret;
        }
    };

    
    
    struct int_indexer{
        template <typename Container>
        inline constexpr static size_t  LinearIndex(Container const& _cnt,size_t i) noexcept{return i;}
        
        template <typename Container>
        inline constexpr static void MultiIncrement(Container const& _cnt,size_t &i)  noexcept{++i;}
        
        template <typename Container>
        inline constexpr static size_t FromLinear (Container const& _cnt,size_t i) noexcept{return i;}
        
        template <typename Container>
        inline constexpr static size_t MultiZero(Container const& _cnt) noexcept{return 0;}
        
        template <typename Container>
        inline constexpr static bool IsEnd(Container const& _cnt,size_t i) noexcept{return i == _cnt.size();}
    };

    /// @brief one-dimention grid managed by Container
    /// @tparam Container type of container
    /// @tparam Helper struct which has static implementations: 
    /// pos_impl, contain_impl, watch class uniform_grid_helper
    /// @tparam Indexer type, contains static multifunctions whatch int_indexer
    template <typename Container,typename Helper,typename Indexer = int_indexer>
    struct Grid1: public Container{
        typedef Container container_t;
        typedef Helper helper_t;
        typedef Indexer indexer_t;
        public:
        inline constexpr  decltype(auto) container() const noexcept{
            return static_cast<Container const &>(*this);
        }
        inline constexpr  decltype(auto) container() noexcept{
            return static_cast<const Container &>(*this);
        }
        public:


        constexpr static size_t Dim =  1;

        typedef Container impl_container;
        using Container::Container;
        typedef typename std::decay<decltype(std::declval<Container>()[0])>::type value_type;
        /// @brief finds placement of x coord
        /// @param x 
        /// @return the index of the nearest to x and less than x point in the grid
        template <typename T>
        constexpr inline size_t pos(T const  &x) const noexcept{
            return Helper::pos_impl(container(),x);
        }

        /// @brief tulpe version of pos(x)
        /// @return pos(std::get<tuple_index>(X))
        template <size_t tuple_index = 0,typename...Args>
        constexpr inline size_t pos_tuple(std::tuple<Args...> const  &X) const noexcept{
            return Helper::pos_impl(*this,std::get<tuple_index>(X));
        }

        /// @brief main constructor of grid from container
        /// @param _cnt base container of grid 
        Grid1 (Container _cnt):Container(std::forward<Container>(_cnt)){}

        template <typename U>
        Grid1(U && u):Container(std::forward < U>(u)){}
        
        /// @brief construct as unifrom grid
        /// @param a 
        /// @param b 
        /// @param m_size size of grid (for std array ignored)
        template <typename T,typename __CNT__ =  Container,
            typename std::enable_if<__default_grid_constructor__<__CNT__>::constructable,bool>::type = true> 
        Grid1(T const & a,T const &  b,size_t m_size=0) 
            noexcept(noexcept(__default_grid_constructor__<Container>::construct(a,b,m_size))):
            Container(__default_grid_constructor__<Container>::construct(a,b,m_size)){}


        template <typename IndexType>
        inline constexpr size_t LinearIndex(IndexType const & i)const noexcept{
            return Indexer::LinearIndex(container(),i);
        }
        
        template <typename IndexType>
        inline constexpr void MultiIncrement(IndexType & i) const noexcept{
            Indexer::MultiIncrement(container(),i);
        }
        
        inline constexpr auto FromLinear (size_t i)const noexcept{
            return Indexer::FromLinear(container(),i);
        }
        
        inline constexpr auto MultiZero()const noexcept{
            return Indexer::MultiZero(container());
        }
        
        inline constexpr bool IsEnd(size_t i)const noexcept{
            return Indexer::IsEnd(container(),i);
        }
        

        /// @brief checks if x in grid
        template <typename T>
        constexpr inline bool contains(const T &x)const{
            return Helper::contain_impl(static_cast<Container const&>(*this),x);
        }

        /// @brief tuple version of contains(x)
        /// @return contains(std::get<tuple_index>(X))
        template <size_t tuple_index = 0,typename...Args>
        inline constexpr bool contains_tuple(std::tuple<Args...> const & X) const noexcept{
            return Helper::contain_impl(static_cast<Container const&>(*this),std::get<tuple_index>(X));
        }

        /// @brief same as pos(x) with check for containing
        /// @tparam U may be a tuple or type convertible to value_type
        /// @param x parametr of pos
        /// @return (true,i) if grid contains x, and (false,any_size_t) otherwise
        template <typename U>
        auto spos(U const & x) const{
            return std::make_tuple(contains(x),pos(x));
        };

        /// @brief tuple version of spos(x)
        template <size_t tuple_index = 0,typename...Args>
        std::tuple<bool,size_t> spos_tuple(std::tuple<Args...>const&X){
            return spos(std::get<tuple_index>(X));
        };
        
        template <size_t tuple_index = 0,typename Tuple,typename Index>
        inline constexpr void fill_index_tuple_save_impl(Tuple const & _Tp,
                                                    Index & index_to_fill,
                                                    bool & b) const noexcept{
            std::tie(b,index_to_fill) = spos(std::get<tuple_index>(_Tp));

        }
        
        

        /// @brief
        friend std::ostream & operator << (std::ostream & os,Grid1 const& Grd){
            std::ostringstream S;
            S << "Grid(";
            stools::print_vector::__print__including__vectors__(S ,static_cast<Container const &>(Grd));
            S << ")";
            return os  << S.str();
        }

        template <typename Serializer>
        auto Serialize(Serializer && S) const{
            return stools::Serialize(static_cast<Container const &>(*this),S);
        }

        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            stools::init_serialize(static_cast<Container &>(*this),Object,S);
        }


        /// @brief writes content with writer WriterStreamType
        template <typename WriterStreamType>
        void write(WriterStreamType && w) const{
            stools::write(static_cast<Container const &>(*this),w);
        }

        /// @brief rewrites object with reader ReaderStreamType
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            stools::init_read(static_cast<Container &>(*this),r);
        }

        template <typename Object,typename DeSerializer>
        static Grid1 DeSerialize(Object const& Obj,DeSerializer && DS){
            return Grid1(stools::DeSerialize<Container>(Obj,DS));
        }
        template <typename Reader>
        static Grid1 read(Reader && r){
            return Grid1(stools::read<Container>(r));
        }
    };

    /// @brief makes grid1 from Containr _cnt
    /// @tparam Container 
    /// @param _cnt 
    /// @return Grid1<Container>
    template <typename helper_cnt_type,typename Container>
    auto make_grid1(Container && _cnt){
        return Grid1<typename std::decay<Container>::type,helper_cnt_type>(
            std::forward<Container>(_cnt));
    }

    template <typename Container>
    struct numerical_histo_container: public Container{
        typedef Container CBase;
        typedef typename CBase::value_type value_type;
        using CBase::CBase;

        decltype(auto) unhisto() &{
            return static_cast<Container &>(*this);
        }
        decltype(auto) unhisto() const&{
            return static_cast<Container const &>(*this);
        }
        
        decltype(auto) unhisto() &&{
            return static_cast<Container &&>(*this);
        }
        template <typename Container2>
        constexpr inline numerical_histo_container(numerical_histo_container<Container2> const & cnt)
            noexcept:CBase(cnt.unhisto()){}
        

        constexpr inline numerical_histo_container(CBase cnt)noexcept:CBase(std::move(cnt)){}
        
        INHERIT_DESERIALIZATOR(CBase,numerical_histo_container)
        INHERIT_SERIALIZATOR(CBase)

        constexpr inline size_t size()const noexcept{
            return CBase::size()-1;
        }
        constexpr inline Rect<value_type> operator[](size_t i) const noexcept{
            return {CBase::operator[](i),CBase::operator[](i+1)};
        }
        inline Rect<value_type>  front()const{
            return (*this)[0];
        }
        inline Rect<value_type>  back()const{
            return (*this)[size()-1];
        }

        inline constexpr size_t MultiSize()const noexcept{return size();}
        inline constexpr size_t IsEnd(size_t i)const noexcept{return i == size();}


        struct numerical_histo_container_iterator : public _iterator_base<typename CBase::const_iterator>,public std::iterator<
                        std::bidirectional_iterator_tag ,
                        Rect<value_type>,   
                        size_t,   
                        Rect<value_type> *, 
                        Rect<value_type>  
        >{
            using _iterator_base<typename CBase::const_iterator>::_iterator_base;
            constexpr inline Rect<value_type> operator *() const noexcept{
                return Rect<value_type>(*this->_base, *(this->_base+ 1) );
            }
            constexpr inline Rect<value_type> operator [](int i) const noexcept{
                return Rect<value_type>(*(this->_base+i), *(this->_base+i+1 ));
            }
        };

        /// @brief  
        inline numerical_histo_container_iterator begin() const noexcept{return CBase::cbegin();}
        /// @brief  
        inline numerical_histo_container_iterator end() const noexcept{return (size() ? CBase::cbegin() + size() : CBase::cend());}
        /// @brief  
        inline numerical_histo_container_iterator cbegin() const noexcept{return begin();}
        /// @brief  
        inline numerical_histo_container_iterator cend() const noexcept{return end();}
    };

    template <typename HistoContainer>
    inline decltype(auto) unhisto(HistoContainer && h_cnt) noexcept{
        return std::forward<HistoContainer>(h_cnt).unhisto();
    }


    template <typename base_helper>
    struct numerical_histo_helper{
        template <typename cnt_type,typename T>
        constexpr inline static  size_t pos_impl(cnt_type const & Grd,T const & x) noexcept{
            return base_helper::pos_impl(unhisto(Grd),x);
        }
        template <typename cnt_type,typename T>
        constexpr inline static bool contain_impl(cnt_type const & Grd,T const & x) noexcept{
            return base_helper::contain_impl(unhisto(Grd),x);
        }
    };


    template <typename Container>
    struct __default_grid_constructor__<numerical_histo_container<Container>>{
        constexpr static bool constructable = __default_grid_constructor__<Container>::constructable;
        template <typename T>
        static inline numerical_histo_container<Container> construct(T const &a,T const & b,size_t size) noexcept{
            return __default_grid_constructor__<Container>::construct(a,b,size);
        }
    };

    template <typename int_type = size_t>
    struct RangeLight{
        typedef int_type value_type;
        size_t _size;
        RangeLight(int_type _size = 0):_size(_size){}
        
        inline constexpr size_t size() const noexcept {return _size;}
        inline constexpr value_type operator [](size_t i) const noexcept{
            return static_cast<value_type>(i);
        }
        struct iterator{
            int_type i;

            inline constexpr iterator (int_type i) noexcept:i(i){}
            inline constexpr operator int_type & () noexcept{
                return i;
            }   
            inline constexpr operator int_type const& () const noexcept{
                return i;
            }
            int_type operator *()const noexcept{
                return i;
            }
        };

        inline constexpr int_type front() const noexcept {return 0;}
        inline constexpr int_type back() const noexcept {return _size-1;}

        inline constexpr iterator begin()const noexcept{
            return 0;
        }
        inline constexpr iterator end() const noexcept{
            return _size;
        }

        friend std::ostream & operator << (std::ostream & os,RangeLight const & RL){          
            if (RL._size < 3) {
                os << "Range(";
                for(size_t i=0;i< RL._size;++i)
                    os << (i !=0 ? ", " : "")<<i; 
                os << ")";
            } else {
                os << "Range(0,1,.., " << RL._size-1 << ")";
            }
            return os;
        }
        SERIALIZATOR_FUNCTION(PROPERTY_NAMES("size"),PROPERTIES(_size))
        WRITE_FUNCTION(_size)
        DESERIALIZATOR_FUNCTION(RangeLight,PROPERTY_NAMES("size"),PROPERTY_TYPES(_size))
        READ_FUNCTION(RangeLight,PROPERTY_TYPES(_size))

    };
    struct rangelight_helper{
        template <typename int_type>
        static inline constexpr size_t pos_impl(RangeLight<int_type> const &_self,int_type x)noexcept{
            return static_cast<size_t>(x);
        }
        template <typename int_type>
        static inline constexpr bool contain_impl(RangeLight<int_type> _self,int_type x)noexcept{
            return 0<=x && x< _self.size;
        }
    };

    template <typename int_type = size_t>
    struct Range{
        typedef int_type value_type;
        int_type start;
        int_type step;
        size_t _size;
        inline constexpr size_t size() const noexcept {return _size;}
        Range(int_type start = 0,int_type step = 1, int_type _size = 0):
            start(start),step(step),_size(_size){}

        inline constexpr value_type operator [](size_t i) const noexcept{
            return static_cast<value_type>(start + i*step);
        }
        struct iterator{
            int_type value;
            int_type step;

            inline constexpr iterator (int_type value,int_type step) noexcept:value(value),step(step){}
            inline iterator & operator ++() noexcept{
                value += step;
            }
            inline iterator operator ++(int) noexcept{
                iterator ret =*this;
                value += step;
                return ret;
            }
            int_type operator *()const noexcept{
                return value;
            }
        };
        constexpr inline int_type front() const noexcept {return start;}
        constexpr inline int_type back() const noexcept {return start+ step*(_size-1);}

        inline constexpr iterator begin()const noexcept{
            return 0;
        }
        inline constexpr iterator end() const noexcept{
            return start + step*_size;
        }

        friend std::ostream & operator << (std::ostream & os,Range const & RL){          
            if (RL._size < 3) {
                os << "Range(";
                for(size_t i=0;i< RL._size;++i)
                    os << (i != 0 ? ", " : "")<<RL[i]; 
                os << ")";
            } else {
                os << "Range(" << RL[0] << ", " << RL[1] << ",.., " << RL[RL._size-1] << ")";
            }
            return os;
        }
        SERIALIZATOR_FUNCTION(PROPERTY_NAMES("start","step","size"),PROPERTIES(start,step,_size))
        WRITE_FUNCTION(start,step,_size)
        DESERIALIZATOR_FUNCTION(Range,PROPERTY_NAMES("start","step","size"),PROPERTY_TYPES(start,step,_size))
        READ_FUNCTION(Range,PROPERTY_TYPES(start,step,_size))

     
   };
    struct range_helper{
        template <typename int_type>
        static inline constexpr size_t pos_impl(Range<int_type> _self,int_type x)noexcept{
            return static_cast<size_t>( (x-_self.start)/_self.step);
        }
        template <typename int_type>
        static inline constexpr bool contain_impl(Range<int_type> _self,int_type x)noexcept{
            int_type Dl = x - _self.start;
            return Dl % _self.step ==0 && (0 <= Dl <  _self.step*_self.size);
        }
    };
    /**
     * \brief pseudo vector of points from a to b of type T
    */
    template <typename T>
    struct UniformContainer{
        protected:
        T a;
        T b;
        T _fac; /// 1/(size-1)
        T _h_1; /// 1/h
        size_t _size;
        friend struct  uniform_grid_helper;
        public:

        typedef T value_type; 

        /// @brief constructor
        /// @param a 
        /// @param b 
        /// @param _size 
        constexpr inline UniformContainer(T  a = 0, T   b = 1,size_t _size = 2)
        noexcept:a(a),b(b),_fac(((T)1)/(_size-1)),_h_1 ((_size-1)/(b-a)),_size(_size){

        }

        template <typename...VectorArgs>
        operator std::vector<VectorArgs...>() const {
            std::vector<VectorArgs...> ret;
            ret.reserve(_size);
            for(size_t i=0;i<_size;++i){
                ret.push_back((*this)[i]);
            }
            return ret;
        }

        template <typename U,size_t N>
        operator std::array<U,N>() const noexcept{
            std::array<U,N> ret;
            for(size_t i=0;i<N;++i){
                ret[i] = (*this)[i];
            }
            return ret;
        } 

        /// @brief copy constructor or converting constructor
        /// @param another uniform container GU 
        template <typename U>
        constexpr inline UniformContainer(UniformContainer<U> const& GU)noexcept:UniformContainer(
            GU.front(),GU.back(),GU.size()
        ){}
        
        
        /// @brief printing to stream
        friend void __print_container__ (std::ostream & os,const UniformContainer & VG){
            std::stringstream internal_stream;
            internal_stream << "Uniform(" << VG._size << ")[" << VG.a <<", "<< VG.b;
            internal_stream << "]";
            os << internal_stream.str();
        }

        

        /// @brief 
        /// @return 
        constexpr inline const T front() const noexcept{return a;}
        
        constexpr inline const T h()const{return (b-a)*_fac;}
        constexpr inline const T h_inv()const{return _h_1;}

        /// @brief 
        /// @return 
        constexpr inline const T back() const noexcept{return b;}
        
        /// @brief 
        /// @return 
        constexpr inline size_t size()const noexcept{
            return _size;
        }

        /// @brief 
        /// @return 
        constexpr inline void resize(size_t N) noexcept{
            _size = N;
            _fac = ((T)1)/(_size-1);
            _h_1 = (_size-1)/(b-a);
        }

        /// @brief accessor to element 
        /// @param i 
        /// @return 
        constexpr inline T operator[](size_t i) const noexcept{
            return (a*(_size-i-1) + i*b)*_fac;
        }

        /// @brief iterator class
        typedef _const_iterator_template<const UniformContainer,T> const_iterator;

        /// @brief 
        /// @return 
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        /// @return 
        inline const_iterator end()const noexcept{return const_iterator(*this,_size);}
        /// @brief 
        /// @return 
        inline const_iterator cbegin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        /// @return 
        inline const_iterator cend()const noexcept{return const_iterator(*this,_size);}


        /// @brief debuging to stream
        friend std::ostream & operator << (std::ostream & os,const UniformContainer & VG){
            __print_container__(os,VG);
            return os;
        }

        SERIALIZATOR_FUNCTION(PROPERTY_NAMES("a","b","size"),PROPERTIES(a,b,_size))
        WRITE_FUNCTION(a,b,_size)
        DESERIALIZATOR_FUNCTION(UniformContainer,PROPERTY_NAMES("a","b","size"),PROPERTY_TYPES(a,b,_size))
        READ_FUNCTION(UniformContainer,PROPERTY_TYPES(a,b,_size))
    };
    struct uniform_grid_helper{
        template <typename T,typename U>
        static constexpr size_t pos_impl(UniformContainer<T> const & _self,U const & x)noexcept{
            if(x <= _self.a)
                return 0;
            else if(x >= _self.b)
                return _self._size-2;
            else
                return static_cast<size_t>( (x-_self.a)*_self._h_1);
        }
        template <typename T,typename U>
        static inline constexpr bool contain_impl(UniformContainer<T> const & _self,U const & x)noexcept{
            return _self.a<=x && x<=_self.b;
        }
    };

    /// @brief container whose values are results of function applied to uniform container
    /// @tparam T value type of container
    /// @tparam FunctypeToHidden any functype with signature T -> T_hidden
    /// @tparam FunctypeFromHidden  any functype with signature T_hidden -> T
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    struct FunctionalContainer{
        typedef UniformContainer<
                typename std::decay<
                        decltype(std::declval<FunctypeToHidden>()(std::declval<T>()))
                    >::type
            > UBase;
        
        
        protected:
        UBase _body;
        friend struct functional_grid_helper;

        FunctypeToHidden _to_hidden;
        FunctypeFromHidden _from_hidden;
        public:
        typedef T value_type;
        typedef typename UBase::value_type  hidden_value_type;
        
        /// @brief constructor of container
        /// @param a begin of range
        /// @param b end of range
        /// @param _size size of range
        /// @param _to_hidden rising function, which maps a..b -> a'..b'
        /// @param _from_hidden rising function, which maps a'..b' -> a..b
        constexpr inline FunctionalContainer( T a,T  b,size_t _size,
                    FunctypeToHidden _to_hidden,FunctypeFromHidden  _from_hidden)noexcept:
                        _body(_to_hidden(a),_to_hidden(b),_size),
                        _to_hidden(std::forward<FunctypeToHidden>(_to_hidden)),
                        _from_hidden(std::forward<FunctypeFromHidden>(_from_hidden)){}
        
        /// @brief constructor of container
        /// @param _uniform_base UniformContainer of range a'..b'
        constexpr inline FunctionalContainer(const UBase &_uniform_base,
                        FunctypeToHidden  _to_hidden,FunctypeFromHidden _from_hidden)noexcept:
                        _to_hidden(std::forward<FunctypeToHidden>(_to_hidden)),
                        _from_hidden(std::forward<FunctypeFromHidden>(_from_hidden)),UBase(_uniform_base){}
        

        constexpr inline hidden_value_type to_hidden(value_type const & external_x)const noexcept{
            return _to_hidden(external_x);
        } 
        constexpr inline hidden_value_type from_hidden(value_type const & internal_x)const noexcept{
            return _from_hidden(internal_x);
        } 
        constexpr inline UBase const & hidden()const{
            return _body;
        }
        constexpr inline UBase & hidden(){
            return _body;
        }

        constexpr inline size_t size() const noexcept{return _body.size();}
        constexpr inline void resize(size_t N)  noexcept{ _body.resize(N);}
        constexpr inline const T front() const noexcept{return _from_hidden(_body.front());}
        constexpr inline const T back() const noexcept{return _from_hidden(_body.back());}
        constexpr inline T operator[](size_t i) const noexcept{
            return _from_hidden(_body[i]);
        }
        
        /// @brief printing to stream
        friend void __print_container__ (std::ostream & os,const FunctionalContainer & VG){
            std::ostringstream internal_stream;
            internal_stream << "FunctionalContainer(" << VG.size() << ")[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ", " <<VG[i];
            }
            internal_stream << "]";
            os << internal_stream.str();
        }

        /// @brief 
        friend std::ostream & operator << (std::ostream & os,const FunctionalContainer & VG){
           __print_container__(os,VG);
           return os;
        }

        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeArray(size(),[this,&S](size_t i){
                return stools::Serialize((*this)[i],S);
            });
        }

        template <typename WriterStreamType>
        void write(WriterStreamType && w) const{
            w.write(size());
            for(size_t i=0;i<size();++i)
                w.write((*this)[i]);
        }


        typedef _const_iterator_template<const FunctionalContainer,T> const_iterator;

        /// @brief 
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        inline const_iterator end()const noexcept{return const_iterator(*this,this->size());}
        /// @brief 
        inline const_iterator cbegin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        inline const_iterator cend()const noexcept{return const_iterator(*this,this->size());}
        
    };

    struct functional_grid_helper{
        template <typename FunctionalContainerType,typename T>
        static constexpr size_t pos_impl(FunctionalContainerType const & _self,T const & x)noexcept{
            return uniform_grid_helper::pos_impl(_self._body,_self._to_hidden(x));
        }
        template <typename FunctionalContainerType,typename T>
        static inline constexpr bool contain_impl(FunctionalContainerType const & _self,const T &x)noexcept{
            return uniform_grid_helper::contain_impl(_self._body,_self._to_hidden(x));
        }
    };

    /// @brief makes Grid1<FunctionContainer>
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    inline auto make_func_grid(T const & a, T const & b,size_t _size,FunctypeToHidden && fth,
                                                FunctypeFromHidden && ffh){
        return make_grid1<functional_grid_helper>(FunctionalContainer<typename std::decay<T>::type,
                    typename std::decay<FunctypeToHidden>::type,
                    typename std::decay<FunctypeFromHidden>::type
                >(a,b,_size,
                    std::forward<FunctypeToHidden>(fth),
                    std::forward<FunctypeFromHidden>(ffh))
        );
    }
    /// @brief makes GridHisto<FunctionContainer>
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    auto make_func_histo_grid(T &&a, T && b,size_t _size,FunctypeToHidden && fth,
                                                    FunctypeFromHidden && ffh){
        return make_grid1<numerical_histo_helper<functional_grid_helper>>(
                FunctionalContainer<typename std::decay<T>::type,
                    typename std::decay<FunctypeToHidden>::type,
                    typename std::decay<FunctypeFromHidden>::type
                >(std::forward<T>(a),std::forward<T>(b),_size,
                    std::forward<FunctypeToHidden>(fth),
                    std::forward<FunctypeFromHidden>(ffh)
                )
        );
    } 
   
    
    template <typename T>
    using GridUniform = Grid1<UniformContainer<T>,uniform_grid_helper>;
    
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    using GridFunctional = Grid1<FunctionalContainer<T,FunctypeToHidden,FunctypeFromHidden>,
                                functional_grid_helper>;

    template <typename...Args>
    using GridVector = Grid1<std::vector<Args...>,vector_array_grid_helper>;

    template <typename T,size_t size>
    using GridArray = Grid1<std::array<T,size>,vector_array_grid_helper>;

    template <typename T>
    using GridUniformHisto = Grid1<numerical_histo_container<UniformContainer<T>>,
                                    numerical_histo_helper<uniform_grid_helper>>;
    
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    using GridFunctionalHisto = Grid1<
                                    numerical_histo_container<
                                        FunctionalContainer<T,FunctypeToHidden,FunctypeFromHidden>
                                    >,
                                    numerical_histo_helper<functional_grid_helper>
                                >;

    template <typename...Args>
    using GridVectorHisto = Grid1<
                                    numerical_histo_container<std::vector<Args...>>,
                                    numerical_histo_helper<vector_array_grid_helper>
                                >;

    template <typename T,size_t size>
    using GridArrayHisto = Grid1<
                                    numerical_histo_container<std::array<T,size>>,
                                    numerical_histo_helper<vector_array_grid_helper>
                                >;

    template <typename int_type = size_t>
    using GridRangeLight = Grid1<
                                    RangeLight<int_type>,
                                    rangelight_helper
                                >;
    
    template <typename int_type = size_t>
    using GridRange = Grid1<
                                Range<int_type>,
                                range_helper
                            >;
    
    

    /*!
        Serializer should contain methods to:
        1) MakePrimitive(T x)
        2) MakeDict(property_num,key_lambda,value_lambda)
        3) MakeArray(size,value_lambda)
    */
    /*!
        DeSerializer should contain methods to:
        1) GetPrimitive(Object,T & value)
        2) GetArraySize(Object)
        3) GetProperty(Object,key)
        4) GetItem(Object,index)
    */
    

};

#endif//GRID_HPP
