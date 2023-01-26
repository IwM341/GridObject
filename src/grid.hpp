#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <arrray>
#include <sstream>

/*!
    \brief implementation and definition of grid
*/
namespace grid_object{
    template <typename GridType>
    struct IsGrid;


    /*!
    \brief uniform grid class
    */
    template <typename T>
    class UniformGrid{
        T a;
        T b;
        size_t _size;

        public:
        typedef T value_type; 
        constexpr static Dim =  1;
        
        constexpr inline UniformGrid(const T & T a = 0,const T & T b = 1,size_t _size = 2)noexcept:a(a),b(b),_size(_size){}
        constexpr inline size_t pos(const T &x) const noexcept {
            if(x <= a)
                return 0;
            else if(x >= b)
                return _size-2;
            else
                return (x-a)/(b-a)*(_size-1);
        }
        constexpr inline size_t size()const noexcept{return _size;}
        
        constexpr inline T operator[](size_t i) const noexcept{
            return a + i*(b-a)*(N-1);
        }

        constexpr inline const T front() const noexcept{return a;}
        constexpr inline T & front() noexcept{return a;}

        constexpr inline const T back() const noexcept{return b;}
        constexpr inline T & back() noexcept{return b;}
        constexpr inline bool contains(const T &x)const{return a<=x<=b;}

        friend std::ostream & operator << (std::ostream & os,const UniformGrid & VG){
            std::ostringstream internal_stream;
            internal_stream << "UniformGrid[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ",\t" <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream;
        }
        constexpr size_t LinearIndex(size_t i)const noexcept{return i;}
        constexpr size_t MultiSize()const noexcept{return _size;}
        constexpr inline void MultiIncrement(size_t & i) const noexcept{++i;}
    };

    template <typename T,typename FunctypeToHidden,typename FunctypeFromHidden>
    class FunctionalGrid:public UniformGrid<T>{
        typedef T value_type;
        typedef UniformGrid<T> Base;
        FunctypeToHidden _to_hidden;
        FunctypeFromHidden _from_hidden;
        
        public:
        constexpr static Dim =  1;
        
        constexpr inline FunctionalGrid(T const &a,T const & b,size_t _size,const FunctypeToHidden &_to_hidden,const FunctypeToHidden &_from_hidden)noexcept:
                        _to_hidden(_to_hidden),_from_hidden(_from_hidden),Base(a,b,_size){}
        constexpr inline FunctionalGrid(const Base &_uniform,const FunctypeToHidden &_to_hidden,const FunctypeToHidden &_from_hidden)noexcept:
                        _to_hidden(_to_hidden),_from_hidden(_from_hidden),Base(_uniform){}
        constexpr inline size_t pos(T const &x) const noexcept {
            return Base::pos(_to_hidden(x));
        }
        constexpr inline T operator[](size_t i) const noexcept{
            return _from_hidden(Base[i]);
        }

        constexpr inline bool contains(const T &x)const{return a<=_to_hidden(x)<=b;}

        constexpr inline const T front() const noexcept{return _from_hidden(a);}

        constexpr inline const T back() const noexcept{return _from_hidden(b);}

        friend std::ostream & operator << (std::ostream & os,const UniformGrid & VG){
            std::ostringstream internal_stream;
            internal_stream << "FunctionalGrid[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ",\t" <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream;
        }
    };

    template <typename T,size_t _size>
    struct ArrayGrid:public std::array<T,_size>{
        typedef T value_type;
        constexpr static Dim =  1;
        typedef std::array<T,_size> Base;
        VectorGrid(T a = 0,T b = 1):{
            for(size_t i=0;i<_size;++i)
                operator[](i) = i*(b-a)/(_size-1);
        }
        constexpr inline ArrayGrid(Base && _array) noexcept:Base(std::move(_array)){}
        constexpr inline ArrayGrid(const Base & _array) noexcept:Base(_array){}

        size_t pos(T x);

        friend std::ostream & operator << (std::ostream & os,const ArrayGrid & VG){
            std::ostringstream internal_stream;
            internal_stream << "ArrayGrid[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ",\t" <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream;
        }
        size_t LinearIndex(size_t i)const{return i;}
        size_t MultiSize()const{return Base.size();}
        inline void MultiIncrement(size_t & i){++i;}

    };

    template <typename T>
    struct VectorGrid:public std::vector<T>{
        typedef T value_type;
        constexpr static Dim =  1;
        typedef std::vector<T> Base;
    

        VectorGrid();

        VectorGrid(T a,T b,size_t _size);
        VectorGrid(Base && _vector):Base(std::move(_vector)){}
        VectorGrid(const Base & _vector):Base(_vector){}
        
        template <typename U>
        VectorGrid(const UniformGrid<U> &_uniform_grid):Base(_uniform_grid.size()){
            size_t N = _uniform_grid.size();
            for(size_t i=0;i<N;++i)
                operator[](i) = _uniform_grid[i];
        }

        template <size_t N,typename U>
        VectorGrid(const ArrayGrid<U,N> &_array_grid):Base(N){
            for(size_t i=0;i<N;++i)
                operator[](i) = _array_grid[i];
        }

        constexpr inline bool contains(const T &x)const{return front()<=x<=back();}

        size_t pos(T x);

        friend std::ostream & operator << (std::ostream & os,const VectorGrid & VG){
            std::ostringstream internal_stream;
            internal_stream << "VectorGrid[";
            if(VG.size()){
                internal_stream << VG[0];
            }
            for(size_t i=1;i<VG.size();++i){
                internal_stream << ",\t" <<VG[i];
            }
            internal_stream << "]";
            return os << internal_stream;
        }
        size_t LinearIndex(size_t i)const{return i;}
        size_t MultiSize()const{return Base.size();}
        inline void MultiIncrement(size_t & i){++i;}
    };

} 

#endif//GRID_HPP