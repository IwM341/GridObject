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

    #ifdef __cpp_concepts
    template <class GridType>
    conscept GridConcept = requires(GridType G){
        G::value_type;
        G.LinearIndex(...);
        G.MultiIncrement(...);
        G.FromLinear(...);
        G.FindIndex(...);
        G.Multi0(...);
        G.contains(...);
        G.pos(...);
    }
    #endif
    #define COMMON_GRID_DEFAULT_FUNCS \
        typedef Rect<T> RectType;\
        typedef Rect<size_t> IndexRectType;\
        typedef Rect<T> ElementType;\
        typedef T PointType;\
        inline constexpr size_t LinearIndex(size_t i)const noexcept{return i;}\
        inline constexpr void MultiIncrement(size_t & i) const noexcept{++i;}\
        inline constexpr size_t FromLinear (size_t i)const noexcept{return i;}\
        inline constexpr size_t Multi0()const noexcept{return 0;}\
        inline Rect<size_t> FindIndex(const T &x) const noexcept{\
            size_t i = this->pos(x);\
            return Rect<size_t>(i,i+1);\
        }\
        template <size_t tuple_index,typename Tuple>\
        inline constexpr bool contains(Tuple const & X) const noexcept{\
            return contains_tuple(std::get<tuple_index>(X));\
        }\
        template <typename U>\
        std::tuple<bool,size_t> spos(U const & x){\
            if(contains(x)){\
                return {true,pos(x)};\
            }\
            else\
                return {false,0};\
        };\
        template <size_t tuple_index,typename...Args>\
        std::tuple<bool,size_t> spos(std::tuple<Args...>const&X){\
            return spos(std::get<tuple_index>(X));\
        };\
        inline std::tuple<Rect<size_t>,Rect<T>> FindElement(const T &x) const noexcept{\
            size_t i = this->pos(x);\
            return std::make_tuple(Rect<size_t>(i,i+1),Rect<T>((*this)[i],(*this)[i+1]));\
        }\
        template <size_t shift,typename...Args>\
        inline void fill_tuple(std::tuple<Args...> & TX,size_t i) const noexcept{\
            std::get<shift>(TX) = (*this)[i];\
        }\
        template <size_t shift,typename...Args>\
        inline void fill_index_tuple_save_impl(std::tuple<Args...> const & TX,bool & b,size_t & i) const noexcept{\
            if(b){\
                b = contains(std::get<shift>(TX));\
                i =  pos(std::get<shift>(TX));\
            }\
        }\
        template <size_t shift,typename...Args>\
        inline void fill_index_tuple_impl(std::tuple<Args...> const & TX,size_t & i) const noexcept{\
            i =  pos(std::get<shift>(TX));\
        }\
        template <size_t shift,typename RectIndex,typename...Args>\
        inline  void fill_rect_index_impl(std::tuple<Args...> const & TX,RectIndex & MI) const noexcept{\
        size_t i = pos(std::get<shift>(TX));\
            MI.left() = i;\
            MI.right() = i+1;\
        }\
        template <size_t shift,typename...Args>\
        inline void fill_element(std::tuple<Args...> const & TX,IndexRectType & IE,RectType & RE) const noexcept{\
            size_t i = pos(std::get<shift>(TX));\
            IE.left() = i;\
            IE.right() = i+1;\
            RE.left() = (*this)[i];\
            RE.right() = (*this)[i+1];\
        }

    #define SPECIAL_GRID_DEFAULT_FUNCS\
        inline constexpr size_t MultiSize()const noexcept{return this->size();}\
        inline constexpr size_t IsEnd(size_t i)const noexcept{return i == this->size();}\

    #define SPECIAL_HISTO_GRID_DEFAULT_FUNCS\
        typedef Rect<T> PointType;\
        inline size_t FindIndex(const T &x) const noexcept{\
                return this->pos(x);\
            }\
        inline std::tuple<size_t,typename GBase::RectType> FindElement(const T &x) const noexcept{\
            size_t i = this->pos(x);\
            return std::make_tuple(i,(*this)[i]);\
        }\
        template <size_t shift,typename ...Args>\
        inline  void fill_element(std::tuple<Args...> const & TX,size_t & I,typename GBase::RectType & RE) const noexcept{\
            size_t i = pos(std::get<shift>(TX));\
            I = i;\
            RE = (*this)[i];\
        }\
        inline void fill_rect(typename GBase::RectType & R,size_t i){\
            R = (*this)[i];\
        }



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
    template <typename GridType>
    constexpr bool __contain__impl__(GridType const & Grd,typename std::decay<decltype(std::declval<GridType>()[0])>::type const & x) noexcept{
        return Grd.first() <= x && x <= Grd.back();
    }

    template <typename GridType>
    constexpr size_t __pos__impl__(GridType const & Grd,typename std::decay<decltype(std::declval<GridType>()[0])>::type const & x) noexcept{
        return __find_index_sorted_with_guess(Grd,x);
    }

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
        static std::vector<T,Args...> construct(T const &a,T const & b,size_t size){
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
        static std::array<T,_size> construct(T const &a,T const & b,size_t size){
            std::array<T,_size> ret;
            for(size_t i=0;i<_size;++i){
                ret[i] = ( a + i*(b-a)/(_size-1));
            }
            return ret;
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
        size_t _size;

        public:

        typedef T value_type; 

        /// @brief constructor
        /// @param a 
        /// @param b 
        /// @param _size 
        constexpr inline UniformContainer(T  a = 0, T   b = 1,size_t _size = 2)noexcept:a(a),b(b),_size(_size){}

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
        /// @tparam Container 
        /// @param GU 
        template <typename Container>
        constexpr inline UniformContainer(Container const& GU)noexcept:a(GU.front()),b(GU.back()),_size(GU.size()){}

        friend constexpr size_t __pos__impl__(UniformContainer const & _self,value_type const & x)noexcept{
            if(x <= _self.a)
                return 0;
            else if(x >= _self.b)
                return _self._size-2;
            else
                return (x-_self.a)/(_self.b-_self.a)*(_self._size-1);
        }

        friend inline constexpr bool __contain__impl__(UniformContainer const & _self,const T &x)noexcept{
            return _self.a<=x && x<=_self.b;
        }
        
        /// @brief printing to stream
        friend void __print_container__ (std::ostream & os,const UniformContainer & VG){
            std::stringstream internal_stream;
            internal_stream << "Uniform(" << VG._size << ")[" << VG.a <<", "<< VG.b;
            internal_stream << "]";
            os << internal_stream.str();
        }

        /// @brief debuging to stream
        friend std::ostream & operator << (std::ostream & os,const UniformContainer & VG){
            __print_container__(os,VG);
            return os;
        }

        

        /// @brief serialize into object such as boost ptree
        /// @tparam Serializer type, translating types to object type
        /// @param S 
        /// @return S.MakeDict(...)
        template <typename Serializer>
        auto Serialize(Serializer && S) const{
            static std::array<const char*,3> fields({"size","a","b"});
            return S.MakeDict(3,[](size_t i){
                    return fields[i];
                },[&S,this](size_t i){
                    switch(i){
                        case 0:
                            return S.MakePrimitive(_size);
                            break;
                        case 1:
                            return S.MakePrimitive(a);
                            break;
                        default:
                            return S.MakePrimitive(b);
                            break;
                    };
                });
        }

        /// @brief rewrite this object from Object with S
        /// @tparam ObjecType 
        /// @tparam DeSerializer class, which can translate ObjecType to any types
        /// @param Object property contating object (e.g. boost ptree)
        /// @param S serializer
        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            S.GetPrimitive(S.GetProperty(Object,"size"),_size);
            S.GetPrimitive(S.GetProperty(Object,"a"),a);
            S.GetPrimitive(S.GetProperty(Object,"b"),b);
        }
        

        /// @brief writes content with writer WriterStreamType
        template <typename WriterStreamType>
        void write(WriterStreamType && w) const{
            w.write(_size);
            w.write(a);
            w.write(b);
        }

        /// @brief rewrites object with reader ReaderStreamType 
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            r.read(_size);
            r.read(a);
            r.read(b);
        }
        OBJECT_DESERIALIZATION_FUNCTION(UniformContainer)
        OBJECT_READ_FUNCTION(UniformContainer)

        /// @brief 
        /// @return 
        constexpr inline const T front() const noexcept{return a;}
        constexpr inline T & front() noexcept{return a;}

        /// @brief 
        /// @return 
        constexpr inline const T back() const noexcept{return b;}
        constexpr inline T & back() noexcept{return b;}
        
        /// @brief 
        /// @return 
        constexpr inline size_t size()const noexcept{
            return _size;
        }

        /// @brief accessor to element 
        /// @param i 
        /// @return 
        constexpr inline T operator[](size_t i) const noexcept{
            return a + i*(b-a)/(_size-1);
        }

        /// @brief iterator class
        typedef _const_iterator_template<UniformContainer,T> const_iterator;

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
    };

    


    /**
     * \brief one-dimention grid managed by Container
     * 
    */
    template <typename Container>
    struct Grid1: public Container{
        constexpr static size_t Dim =  1;
        using Container::Container;
        typedef typename std::decay<decltype(std::declval<Container>()[0])>::type value_type;

        /// @brief finds placement of x coord
        /// @param x 
        /// @return the index of the nearest to x and less than x point in the grid
        constexpr inline size_t pos(value_type const  &x) const noexcept{
            return __pos__impl__(*this,x);
        }

        /// @brief tulpe version of pos(x)
        /// @return pos(std::get<tuple_index>(X))
        template <size_t tuple_index = 0,typename...Args>
        constexpr inline size_t pos(std::tuple<Args...> const  &X) const noexcept{
            return __pos__impl__(*this,std::get<tuple_index>(X));
        }

        /// @brief main constructor of grid from container
        /// @param _cnt base container of grid 
        Grid1 (Container _cnt):Container(std::forward<Container>(_cnt)){}
        
        template <typename __CNT__ =  Container,
            typename std::enable_if<__default_grid_constructor__<__CNT__>::constructable,bool>::type = true> 
        Grid1(value_type const & a,value_type const & b,size_t _size) 
            noexcept(noexcept(__default_grid_constructor__<Container>::construct(a,b,_size))):
            Container(__default_grid_constructor__<Container>::construct(a,b,_size)){}


        typedef Rect<value_type> RectType;
        typedef Rect<size_t> IndexRectType;
        typedef Rect<value_type> ElementType;
        typedef value_type PointType;

        inline constexpr size_t LinearIndex(size_t i)const noexcept{return i;}
        inline constexpr void MultiIncrement(size_t & i) const noexcept{++i;}
        inline constexpr size_t FromLinear (size_t i)const noexcept{return i;}
        inline constexpr size_t Multi0()const noexcept{return 0;}

        /// @brief gives pair (i,i+1), where i = pos(x)
        /// @param x 
        /// @return 
        inline Rect<size_t> FindIndex(const value_type &x) const noexcept{
            size_t i = pos(x);
            return Rect<size_t>(i,i+1);
        }

        /// @brief checks if x in grid
        constexpr inline bool contains(const value_type &x)const{
            return __contain__impl__(*this,x);
        }

        /// @brief tuple version of contains(x)
        /// @return contains(std::get<tuple_index>(X))
        template <size_t tuple_index = 0,typename...Args>
        inline constexpr bool contains(std::tuple<Args...> const & X) const noexcept{
            return __contain__impl__(*this,std::get<tuple_index>(X));
        }

        /// @brief same as pos(x) with check for containing
        /// @tparam U may be a tuple or type convertible to value_type
        /// @param x parametr of pos
        /// @return (true,i) if grid contains x, and (false,any_size_t) otherwise
        template <typename U>
        std::tuple<bool,size_t> spos(U const & x){
            if(contains(x)){
                return {true,pos(x)};
            }
            else
                return {false,0};
        };

        /// @brief tuple version of spos(x)
        template <size_t tuple_index = 0,typename...Args>
        std::tuple<bool,size_t> spos(std::tuple<Args...>const&X){
            return spos(std::get<tuple_index>(X));
        };
        
        /// @brief returns indexes and values of interval which contains x
        /// @param x - coord
        /// @return ( [i,i+1],[x_i,x_{i+1}] )
        inline std::tuple<Rect<size_t>,Rect<value_type>> FindElement(const value_type &x) const noexcept{
            size_t i = this->pos(x);
            return std::make_tuple(Rect<size_t>(i,i+1),Rect<value_type>((*this)[i],(*this)[i+1]));
        }

        template <size_t shift,typename...Args>
        inline void fill_tuple(std::tuple<Args...> & TX,size_t i) const noexcept{
            std::get<shift>(TX) = (*this)[i];
        }

        template <size_t shift,typename...Args>
        inline void fill_index_tuple_save_impl(std::tuple<Args...> const & TX,bool & b,size_t & i) const noexcept{\
            if(b){
                b = contains(std::get<shift>(TX));
                i =  pos(std::get<shift>(TX));
            }
        }

        template <size_t shift,typename...Args>
        inline void fill_index_tuple_impl(std::tuple<Args...> const & TX,size_t & i) const noexcept{
            i =  pos(std::get<shift>(TX));
        }
        template <size_t shift,typename RectIndex,typename...Args>
        inline  void fill_rect_index_impl(std::tuple<Args...> const & TX,RectIndex & MI) const noexcept{
        size_t i = pos(std::get<shift>(TX));
            MI.left() = i;
            MI.right() = i+1;
        }
        template <size_t shift,typename...Args>
        inline void fill_element(std::tuple<Args...> const & TX,IndexRectType & IE,RectType & RE) const noexcept{
            size_t i = pos(std::get<shift>(TX));
            IE.left() = i;
            IE.right() = i+1;
            RE.left() = (*this)[i];
            RE.right() = (*this)[i+1];
        }
    };

    /// @brief makes grid1 from Containr _cnt
    /// @tparam Container 
    /// @param _cnt 
    /// @return Grid1<Container>
    template <typename Container>
    auto make_grid1(Container && _cnt){
        return Grid1<typename std::decay<Container>::type>(
            std::forward<Container>(_cnt));
    }

    template <typename Container>
    std::ostream & operator << (std::ostream & os,Grid1<Container> const & VG){
        __print_container__(os,static_cast<Container const &>(VG));
        return os;
    }


    template <typename Container>
    struct GridHisto: public Grid1<Container>{
        typedef Grid1<Container> GBase; 
        typedef typename GBase::value_type value_type;
        using GBase::GBase;

        constexpr inline GridHisto(GBase const& GU)noexcept:GBase(GU){}
        
        INHERIT_SERIALIZATOR(GBase,GridHisto)
        INHERIT_READ(GBase,GridHisto)

        constexpr inline size_t size()const noexcept{
            return (GBase::_size ? GBase::_size-1 : 0);
        }
        constexpr inline Rect<value_type> operator[](size_t i) const noexcept{
            return {GBase::operator[](i),GBase::operator[](i+1)};
        }

        typedef  _const_iterator_template<GridHisto,Rect<value_type>> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,size());}

        inline std::tuple<size_t,typename GBase::RectType> FindElement(const value_type &x) const noexcept{
            size_t i = this->pos(x);
            return std::make_tuple(i,(*this)[i]);
        }


        template <size_t shift,typename ...Args>
        inline  void fill_element(std::tuple<Args...> const & TX,size_t & I,typename GBase::RectType & RE) const noexcept{
            size_t i = pos(std::get<shift>(TX));
            I = i;
            RE = (*this)[i];
        }
        inline void fill_rect(typename GBase::RectType & R,size_t i){
            R = (*this)[i];
        }

        inline constexpr size_t MultiSize()const noexcept{return size();}
        inline constexpr size_t IsEnd(size_t i)const noexcept{return i == size();}
    };

    /// @brief makes histo_grid1 from Containr _cnt
    /// @tparam Container 
    /// @param _cnt 
    /// @return GridHisto<Container>
    template <typename Container>
    auto make_histo_grid(Container && _cnt){
        return GridHisto<typename std::decay<Container>::type>(
            std::forward<Container>(_cnt));
    }
    
    /// @brief container whose values are results of function applied to uniform container
    /// @tparam T value type of container
    /// @tparam FunctypeToHidden any functype with signature T -> T_hidden
    /// @tparam FunctypeFromHidden  any functype with signature T_hidden -> T
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    struct FunctionalContainer{
        typedef UniformContainer<
                typename std::decay<
                        typename std::invoke_result<FunctypeToHidden,T>::type
                    >::type
            > UBase;
        
        UBase _body;
        protected:
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
        


        constexpr inline size_t size() const noexcept{return _body.size();}
        constexpr inline const T front() const noexcept{return _from_hidden(UBase::a);}
        constexpr inline const T back() const noexcept{return _from_hidden(UBase::b);}
        constexpr inline T operator[](size_t i) const noexcept{
            return _from_hidden(_body[i]);
        }

        friend constexpr size_t __pos__impl__(FunctionalContainer const & _self,value_type const & x)noexcept{
            return __pos__impl__(_self._body,_self._to_hidden(x));
        }

        friend inline constexpr bool __contain__impl__(FunctionalContainer const & _self,const T &x)noexcept{
            return __contain__impl__(_self._body,_self._to_hidden(x));
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
        auto Serialize(Serializer && S){
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


        typedef _const_iterator_template<FunctionalContainer,T> const_iterator;

        /// @brief 
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        inline const_iterator end()const noexcept{return const_iterator(*this,this->_size);}
        /// @brief 
        inline const_iterator cbegin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        inline const_iterator cend()const noexcept{return const_iterator(*this,this->_size);}
    };

    /// @brief makes Grid1<FunctionContainer>
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    auto make_func_grid1(T &&a, T && b,size_t _size,FunctypeToHidden && fth,
                                                FunctypeFromHidden && ffh){
        return make_grid1(FunctionalContainer<typename std::decay<T>::type,
                    typename std::decay<FunctypeToHidden>::type,
                    typename std::decay<FunctypeFromHidden>::type
                >(std::forward<T>(a),std::forward<T>(b),_size,
                    std::forward<FunctypeToHidden>(fth),
                    std::forward<FunctypeFromHidden>(ffh))
        );
    }
    /// @brief makes GridHisto<FunctionContainer>
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    auto make_func_histo_grid(T &&a, T && b,size_t _size,FunctypeToHidden && fth,
                                                    FunctypeFromHidden && ffh){
        return make_grid_histo(
                FunctionalContainer<typename std::decay<T>::type,
                    typename std::decay<FunctypeToHidden>::type,
                    typename std::decay<FunctypeFromHidden>::type
                >(std::forward<T>(a),std::forward<T>(b),_size,
                    std::forward<FunctypeToHidden>(fth),
                    std::forward<FunctypeFromHidden>(ffh)
                )
        );
    } 
   

    template <typename Type>
    struct is_grid1{
        constexpr static bool value = false;
    };
    template <typename Type>
    struct is_grid{
        constexpr static bool value = false;
    };
    template <typename Type>
    struct is_histo_grid{
        constexpr static bool value = false;
    };

    template <typename Container>
    struct is_grid1<Grid1<Container>>{
        constexpr static bool value = true;
    };
    template <typename Container>
    struct is_grid1<GridHisto<Container>>{
        constexpr static bool value = true;
    };

    template <typename Container>
    struct is_grid<Grid1<Container>>{
        constexpr static bool value = true;
    };
    template <typename Container>
    struct is_grid<GridHisto<Container>>{
        constexpr static bool value = true;
    };
    template <typename Container>
    struct is_histo_grid<GridHisto<Container>>{
        constexpr static bool value = true;
    };

    /*!
    \brief uniform grid class
    */
    template <typename T>
    struct GridUniform{
        protected:
        T a;
        T b;
        size_t _size;

        public:
        typedef T value_type; 
        constexpr static size_t Dim =  1;
        
        constexpr inline GridUniform(T  a = 0, T   b = 1,size_t _size = 2)noexcept:a(a),b(b),_size(_size){}

        template <typename GridType>
        constexpr inline GridUniform(GridType const& GU)noexcept:a(GU.front()),b(GU.back()),_size(GU.size()){}

        constexpr inline size_t pos(T const  &x) const noexcept {
            if(x <= a)
                return 0;
            else if(x >= b)
                return _size-2;
            else
                return (x-a)/(b-a)*(_size-1);
        }

        COMMON_GRID_DEFAULT_FUNCS
        SPECIAL_GRID_DEFAULT_FUNCS

        constexpr inline bool contains(const T &x)const{return a<=x && x<=b;}

        friend std::ostream & operator << (std::ostream & os,const GridUniform & VG){
            std::stringstream internal_stream;
            internal_stream << "GridUniform(" << VG._size << ")[" << VG.a <<", "<< VG.b;
            internal_stream << "]";
            return os << internal_stream.str();
        }


        template <typename Serializer>
        auto Serialize(Serializer && S) const{
            static std::array<const char*,3> fields({"size","a","b"});
            return S.MakeDict(3,[](size_t i){
                    return fields[i];
                },[&S,this](size_t i){
                    switch(i){
                        case 0:
                            return S.MakePrimitive(_size);
                            break;
                        case 1:
                            return S.MakePrimitive(a);
                            break;
                        default:
                            return S.MakePrimitive(b);
                            break;
                    };
                });
        }
        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            S.GetPrimitive(S.GetProperty(Object,"size"),_size);
            S.GetPrimitive(S.GetProperty(Object,"a"),a);
            S.GetPrimitive(S.GetProperty(Object,"b"),b);
        }
        

        template <typename WriterStreamType>
        void write(WriterStreamType && w) const{
            w.write(_size);
            w.write(a);
            w.write(b);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            r.read(_size);
            r.read(a);
            r.read(b);
        }
        OBJECT_DESERIALIZATION_FUNCTION(GridUniform)
        OBJECT_READ_FUNCTION(GridUniform)

        constexpr inline const T front() const noexcept{return a;}
        constexpr inline T & front() noexcept{return a;}

        constexpr inline const T back() const noexcept{return b;}
        constexpr inline T & back() noexcept{return b;}
        
        constexpr inline size_t size()const noexcept{
            return _size;
        }
        constexpr inline T operator[](size_t i) const noexcept{
            return a + i*(b-a)/(_size-1);
        }

        typedef _const_iterator_template<GridUniform,T> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,_size);}

        
    };

    template <typename T>
    struct is_grid1<GridUniform<T>>{
        constexpr static bool value = true;
    };
    template <typename T>
    struct is_grid<GridUniform<T>>{
        constexpr static bool value = true;
    };
    
    #define HISTO_ITERATOR_FUNCTIONS\
    struct iterator : public _iterator_base<typename GBase::iterator>, public std::iterator<\
                        std::bidirectional_iterator_tag ,\
                        Rect<T&>,   \
                        size_t,   \
                        Rect<T&> *, \
                        Rect<T&>  \
    >{\
        using _iterator_base<typename GBase::iterator>::_iterator_base;\
        inline Rect<T&> operator *() const noexcept{\
            return Rect<T&>(*this->_base, *(this->_base+ 1) );\
        }\
        inline Rect<T&> operator [](int i) const noexcept{\
            return Rect<T&>(*(this->_base+i), *(this->_base+i+1) );\
        }\
    };\
    struct const_iterator : public _iterator_base<typename GBase::const_iterator>,public std::iterator<\
                        std::bidirectional_iterator_tag ,\
                        Rect<T>,   \
                        size_t,   \
                        Rect<T> *, \
                        Rect<T>  \
    >{\
        using _iterator_base<typename GBase::const_iterator>::_iterator_base;\
        constexpr inline Rect<T> operator *() const noexcept{\
            return Rect<T>(*this->_base, *(this->_base+ 1) );\
        }\
        constexpr inline Rect<T> operator [](int i) const noexcept{\
            return Rect<T>(*(this->_base+i), *(this->_base+i+1 ));\
        }\
    };\
    inline iterator begin() noexcept{return GBase::begin();}\
    inline iterator end() noexcept{return (size() ? GBase::begin() + size() : GBase::end());}\
    inline const_iterator begin() const noexcept{return cbegin();}\
    inline const_iterator end() const noexcept{return cend();}\
    inline const_iterator cbegin() const noexcept{return GBase::cbegin();}\
    inline const_iterator cend() const noexcept{return (size() ? GBase::cbegin() + size() : GBase::cend());}
    
    template <typename T>
    struct GridUniformHisto: public GridUniform<T>{
        typedef GridUniform<T> GBase; 
        using GBase::GridUniform;

        template <typename U>
        constexpr inline GridUniformHisto(GridUniform<U> const& GU)noexcept:GBase(GU){}
        
        INHERIT_SERIALIZATOR(GBase,GridUniformHisto)
        INHERIT_READ(GBase,GridUniformHisto)

        constexpr inline size_t size()const noexcept{
            return (GBase::_size ? GBase::_size-1 : 0);
        }
        constexpr inline Rect<T> operator[](size_t i) const noexcept{
            return {GBase::operator[](i),GBase::operator[](i+1)};
        }

        typedef  _const_iterator_template<GridUniformHisto,Rect<T>> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,size());}

        SPECIAL_GRID_DEFAULT_FUNCS
        SPECIAL_HISTO_GRID_DEFAULT_FUNCS
    };

    /*
    template <typename GridType>
    struct GridHisto{
        typedef GridType GBase;
        using GBase::GBase;

        inline constexpr GridHisto(GBase Base) noexcept: GBase(std::move(Base)){}

        constexpr inline size_t size()const noexcept{
            return (GBase::size() ? GBase::size()-1 : 0);
        }
        constexpr inline Rect<T> operator[](size_t i) const noexcept{
            return {GBase::operator[](i),GBase::operator[](i+1)};
        }

        typedef  _const_iterator_template<GridHisto,Rect<T>> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,size());}


        inline constexpr size_t MultiSize()const noexcept{return this->size();}

        template <typename Object,typename DeSerializer>
        inline static GridHisto DeSerialize(Object && Obj,DeSerializer && DS){
            return GBase::DeSerializer(std::forward<Object>(Obj),std::forward<DeSerializer>(DS));
        }
        template <typename Reader>
        inline static GridHisto read(Reader && r){
            return GBase::read(r);
        }
    };
    */
    template <typename T>
    struct is_grid1<GridUniformHisto<T>>{
        constexpr static bool value = true;
    };
    template <typename T>
    struct is_grid<GridUniformHisto<T>>{
        constexpr static bool value = true;
    };
    template <typename T>
    struct is_histo_grid<GridUniformHisto<T>>{
        constexpr static bool value = true;
    };

    #define DEFAULT_SERIALIZATION_FUNCTIONS \
        template <typename Serializer>\
        auto Serialize(Serializer && S) const{\
            return S.MakeArray(this->size(),[this,&S](size_t i){return S.MakePrimitive((*this)[i]);});\
        }\
        template <typename WriterStreamType>\
        void write(WriterStreamType && w)const{\
            w.write(this->size());\
            for(size_t i=0;i<this->size();++i){\
                w.write((*this)[i]);\
            }\
        }

    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    struct GridFunctional: public GridUniform<typename std::invoke_result<FunctypeToHidden,T>::type>{
    
        protected:
        FunctypeToHidden _to_hidden;
        FunctypeFromHidden _from_hidden;
        
        public:
        typedef T value_type;
        typedef GridUniform<T> UBase;
        typedef typename std::invoke_result<FunctypeToHidden,T>::type  hidden_value_type;
        constexpr static size_t Dim =  1;
        
        constexpr inline GridFunctional( T a,T  b,size_t _size,
                    FunctypeToHidden _to_hidden,FunctypeFromHidden  _from_hidden)noexcept:
                        UBase(_to_hidden(a),_to_hidden(b),_size),
                        _to_hidden(std::forward<FunctypeToHidden>(_to_hidden)),
                        _from_hidden(std::forward<FunctypeFromHidden>(_from_hidden)){}
        constexpr inline GridFunctional(const UBase &_uniform,
                        FunctypeToHidden  _to_hidden,FunctypeFromHidden _from_hidden)noexcept:
                        _to_hidden(std::forward<FunctypeToHidden>(_to_hidden)),
                        _from_hidden(std::forward<FunctypeFromHidden>(_from_hidden)),UBase(_uniform){}
        

    
        constexpr inline size_t pos(T const &x) const noexcept {
            return UBase::pos(_to_hidden(x));
        }
        

        constexpr inline bool contains(const T &x)const{return UBase::a<=_to_hidden(x)<=UBase::b;}


        
        constexpr inline const T front() const noexcept{return _from_hidden(UBase::a);}

        constexpr inline const T back() const noexcept{return _from_hidden(UBase::b);}
        
        friend std::ostream & operator << (std::ostream & os,const GridFunctional & VG){
            std::ostringstream internal_stream;
            internal_stream << "GridFunctional(" << VG._size << ")[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ", " <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream.str();
        }

        DEFAULT_SERIALIZATION_FUNCTIONS


        COMMON_GRID_DEFAULT_FUNCS

        
        constexpr inline T operator[](size_t i) const noexcept{
            return _from_hidden(UBase::operator[](i));
        }

        SPECIAL_GRID_DEFAULT_FUNCS

        typedef _const_iterator_template<GridFunctional,T> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,this->_size);}
    };



    template <typename...Args>
    struct is_grid1<GridFunctional<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_grid<GridFunctional<Args...>>{
        constexpr static bool value = true;
    };
    
    
    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    struct GridFunctionalHisto: public GridFunctional<T,FunctypeToHidden,FunctypeFromHidden>{
        typedef GridFunctional<T,FunctypeToHidden,FunctypeFromHidden> GBase; 
        using GBase::GBase;
 
        constexpr inline size_t size()const noexcept{
            return (GBase::_size ? GBase::_size-1 : 0);
        }
        constexpr inline Rect<T> operator[](size_t i) const noexcept{
            return {GBase::operator[](i),GBase::operator[](i+1)};
        }

        GridFunctionalHisto (const GBase & GB):GBase(GB){}

        SPECIAL_GRID_DEFAULT_FUNCS
        SPECIAL_HISTO_GRID_DEFAULT_FUNCS

        typedef _const_iterator_template<GridFunctionalHisto,Rect<T>> const_iterator;
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        inline const_iterator end()const noexcept{return const_iterator(*this,size());}
    };

    
    template <typename...Args>
    struct is_grid1<GridFunctionalHisto<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_grid<GridFunctionalHisto<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_histo_grid<GridFunctionalHisto<Args...>>{
        constexpr static bool value = true;
    };

    
    template <typename T,size_t _size>
    struct GridArray:public std::array<T,_size>{
        typedef T value_type;
        constexpr static size_t Dim =  1;
        typedef std::array<T,_size> Base;
        
        using Base::Base;
        COMMON_GRID_DEFAULT_FUNCS
        GridArray(T const&a = 0,T const&b = 1){
            for(size_t i=0;i<_size;++i)
                Base::operator[](i) = a + i*(b-a)/(_size-1);
        }
        inline size_t pos(T const&x)const noexcept{
            return __find_index_sorted_with_guess(*this,x);
        }

        friend std::ostream & operator << (std::ostream & os,const GridArray & VG){
            std::ostringstream internal_stream;
            internal_stream << "GridArray(" << _size << ")[    ";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ", " <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream.str();
        }
        DEFAULT_SERIALIZATION_FUNCTIONS

        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            stools::init_serialize(static_cast<Base &>(*this),Object,S);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            size_t r_size;
            r.read(r_size);
            if(r_size != _size){
                throw std::range_error("error in static GridArray read(ReaderStreamType && r)");
            }
            for(size_t i=0;i< _size;++i){
                r.read((*this)[i]);
            }
        }
        OBJECT_DESERIALIZATION_FUNCTION(GridArray)
        OBJECT_READ_FUNCTION(GridArray)


        
    };

    
    template <typename T,size_t _size>
    struct is_grid1<GridArray<T,_size>>{
        constexpr static bool value = true;
    };
    template <typename T,size_t _size>
    struct is_grid<GridArray<T,_size>>{
        constexpr static bool value = true;
    };

    template <typename T,size_t _size>
    struct GridArrayHisto:public GridArray<T,_size>{
        typedef GridArray<T,_size> GBase;
        using GBase::GBase;

        GridArrayHisto(GBase const&GB):GBase(GB){}

        constexpr inline size_t size()const noexcept{
            return GBase::size()-1;
        }
        constexpr inline Rect<T> operator[](size_t i) const noexcept{
            return {GBase::operator[](i),GBase::operator[](i+1)};
        }

        OBJECT_DESERIALIZATION_FUNCTION(GridArrayHisto)
        OBJECT_READ_FUNCTION(GridArrayHisto)

        SPECIAL_GRID_DEFAULT_FUNCS
        SPECIAL_HISTO_GRID_DEFAULT_FUNCS

        HISTO_ITERATOR_FUNCTIONS
        
    };

    template <typename T,size_t _size>
    struct is_grid1<GridArrayHisto<T,_size>>{
        constexpr static bool value = true;
    };
    template <typename T,size_t _size>
    struct is_grid<GridArrayHisto<T,_size>>{
        constexpr static bool value = true;
    };

    template <typename T,size_t _size>
    struct is_histo_grid<GridArrayHisto<T,_size>>{
        constexpr static bool value = true;
    };


    template <typename T,typename...VecParams>
    struct GridVector:public std::vector<T,VecParams...>{;
        //typedef T value_type;
        constexpr static size_t Dim =  1;
        typedef std::vector<T,VecParams...> Base;
        using Base::Base;

        COMMON_GRID_DEFAULT_FUNCS
        SPECIAL_GRID_DEFAULT_FUNCS

        GridVector(T a,T b,size_t _size):Base(_size){
            for(size_t i=0;i<_size;++i)
                Base::operator[](i) = a + (b-a)*i/(_size-1);
        }
        
        template <typename U>
        GridVector(const GridUniform<U> &_uniform_grid):Base(_uniform_grid.size()){
            size_t N = _uniform_grid.size();
            for(size_t i=0;i<N;++i)
                Base::operator[](i) = _uniform_grid[i];
        }

        template <typename U,typename...Args>
        GridVector(const GridFunctional<U,Args...> &_function_grid):Base(_function_grid.size()){
            size_t N = _function_grid.size();
            for(size_t i=0;i<N;++i)
                Base::operator[](i) = _function_grid[i];
        }

        template <size_t N,typename U>
        GridVector(const GridArray<U,N> &_array_grid):Base(_array_grid.begin(),_array_grid.end()){
        }

        constexpr inline bool contains(const T &x)const{return Base::front()<=x<=Base::back();}

        inline size_t pos(T const&x)const noexcept{
            return __find_index_sorted_with_guess(*this,x);
        }

        friend std::ostream & operator << (std::ostream & os,const GridVector & VG){
            std::ostringstream internal_stream;
            internal_stream << "GridVector(" << VG.size() << ")[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ", " <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream.str();
        }
        DEFAULT_SERIALIZATION_FUNCTIONS

        template <typename ObjecType,typename DeSerializer>
        inline void init_serialize(ObjecType && Object,DeSerializer && S){
            stools::init_serialize(static_cast<Base &>(*this),Object,S);
        }
        template <typename ReaderStreamType>
        inline void init_read(ReaderStreamType && r){
            size_t r_size;
            this->resize(r_size);
            for(size_t i=0;i< this->size();++i){
                r.read((*this)[i]);
            }
        }
        OBJECT_DESERIALIZATION_FUNCTION(GridVector)
        OBJECT_READ_FUNCTION(GridVector)


        
    };

    template <typename...Args>
    struct is_grid1<GridVector<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_grid<GridVector<Args...>>{
        constexpr static bool value = true;
    };

    template <typename T,typename...VecParams>
    struct GridVectorHisto:public GridVector<T,VecParams...>{
        typedef GridVector<T,VecParams...> GBase;
        using GBase::GBase;


        constexpr inline GridVectorHisto(GBase GB):GBase(std::move(GB)){}
        constexpr inline size_t size()const noexcept{
            return GBase::size()-1;
        }
        constexpr inline Rect<T> operator[](size_t i) const noexcept{
            return {GBase:: operator[](i),GBase:: operator[](i+1)};
        }

        INHERIT_SERIALIZATOR(GBase,GridVectorHisto)
        INHERIT_READ(GBase,GridVectorHisto)

        SPECIAL_GRID_DEFAULT_FUNCS
        SPECIAL_HISTO_GRID_DEFAULT_FUNCS

        HISTO_ITERATOR_FUNCTIONS

    };


    
    template <typename...Args>
    struct is_grid1<GridVectorHisto<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_grid<GridVectorHisto<Args...>>{
        constexpr static bool value = true;
    };
    template <typename...Args>
    struct is_histo_grid<GridVectorHisto<Args...>>{
        constexpr static bool value = true;
    };



    template <typename GridType>
    struct GridHistoType;

    #define GTHT_TEMPLATE(GridTypeName,GridTypeHistoType)\
    template <typename...Args>\
    struct GridHistoType<GridTypeName<Args...>>{\
        typedef GridTypeHistoType<Args...> type;\
    };

    GTHT_TEMPLATE(GridUniform,GridUniformHisto)
    GTHT_TEMPLATE(GridUniformHisto,GridUniformHisto)

    GTHT_TEMPLATE(GridVector,GridVectorHisto)
    GTHT_TEMPLATE(GridVectorHisto,GridVectorHisto)

    GTHT_TEMPLATE(GridFunctional,GridFunctionalHisto)
    GTHT_TEMPLATE(GridFunctionalHisto,GridFunctionalHisto)

    template <typename T,size_t _size>
    struct GridHistoType<GridArray<T,_size>>{\
        typedef GridArrayHisto<T,_size> type;\
    };
    template <typename T,size_t _size>
    struct GridHistoType<GridArrayHisto<T,_size>>{\
        typedef GridArrayHisto<T,_size> type;\
    };

    template <typename GridType>
    inline constexpr auto HistoCast(GridType &&x) noexcept{
        return static_cast<
                typename templdefs::forward_qualifiers< 
                    GridType &&,typename GridHistoType<
                            typename templdefs::remove_all<GridType>::type
                        >::type
                    >::type
                >(x);
    };
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
