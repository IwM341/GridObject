#ifndef GRID_H
#define GRID_H

#include <vector>
#include <arrray>
///
    \brief implementation and definition of grid
///
namespace grid_object{
    template <typename GridType>
    struct is_grid;


    template <typename T>
    struct UniformGrid{
        constexpr static Dim =  1;
        T a;
        T b;
        size_t _size;
        inline UniformGrid(T a = 0,T b = 1,size_t _size = 2):a(a),b(b),_size(_size){}
        inline size_t pos(T x) const {
            if(x <= a)
                return 0;
            else if(x >= b)
                return _size-2;
            else
                return (x-a)/(b-a)*(_size-1);
        }
        inline size_t size()const{return _size;}
        
        inline T operator[](size_t i){
            return a + i*(b-a)*(N-1);
        }

        inline const T front() const{return a;}
        inline T & front(){return a;}

        inline const T back(){return b;}
        inline T & back(){return b;}

        friend std::ostream & operator << (std::ostream & os,const UniformGrid & VG){
            os << "UniformGrid[";
            os << VG[0];
            for(size_t i=1;i<VG.size();++i){
                os << ",\t" <<VG[i];
            }
            os << "]";
            return os;
        }
        size_t LinearIndex(size_t i)const{return i;}
        size_t MultiSize()const{return _size;}
        inline void MultiIncrement(size_t & i){++i;}
    };

    template <typename T,size_t _size>
    struct ArrayGrid:public std::array<T,_size>{
        constexpr static Dim =  1;
        typedef std::array<T,_size> Base;
        VectorGrid(T a = 0,T b = 1):{
            for(size_t i=0;i<_size;++i)
                operator[](i) = i*(b-a)/(_size-1);
        }
        ArrayGrid(Base && _array):Base(std::move(_array)){}
        ArrayGrid(const Base & _array):Base(_array){}

        size_t pos(T x);

        friend std::ostream & operator << (std::ostream & os,const ArrayGrid & VG){
            os << "ArrayGrid[";
            os << VG[0];
            for(size_t i=1;i<VG.size();++i){
                os << ",\t" <<VG[i];
            }
            os << "]";
            return os;
        }
        size_t LinearIndex(size_t i)const{return i;}
        size_t MultiSize()const{return Base.size();}
        inline void MultiIncrement(size_t & i){++i;}

    };

    template <typename T>
    struct VectorGrid:public std::vector<T>{
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

        size_t pos(T x);

        friend std::ostream & operator << (std::ostream & os,const VectorGrid & VG){
            os << "VectorGrid[";
            os << VG[0];
            for(size_t i=1;i<VG.size();++i){
                os << ",\t" <<VG[i];
            }
            os << "]";
            return os;
        }
        size_t LinearIndex(size_t i)const{return i;}
        size_t MultiSize()const{return Base.size();}
        inline void MultiIncrement(size_t & i){++i;}
    };

} 

#endif//GRID_H