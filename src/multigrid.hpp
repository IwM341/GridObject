#ifndef MULTIDIM_GRID_H
#define MULTIDIM_GRID_H
#include "grid.h"

namespace grid_object{
    
    /*!
        \brief Multiindex class for iterating over multidimension grids
        MultiIndex for N = 1 is size_t
    */
    template <size_t N>
    struct MultiIndex{
        size_t i;
        MultiIndex<N-1> m;
        
        constexpr inline MultiIndex(size_t j=0) noexcept:i(0),m(j){}
        constexpr inline MultiIndex& operator = (size_t j)noexcept{
            i=0;
            m = j;
        }
        constexpr inline bool operator !=(const MultiIndex& mi )const noexcept{
            return (i!=m || m!= mi.m);
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
        constexpr friend inline operator >(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<mi_1;
        }
        constexpr friend inline operator >=(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<=mi_1;
        }
        template <typename...Inners>
        constexpr inline MultiIndex(size_t i,Inners...inn) noexcept:i(0),m(inn...){}
    };

    template <>
    struct MultiIndex<1>{
        size_t i;
        constexpr inline MultiIndex<1>(size_t i=0) noexcept:i(i){}
        constexpr inline operator size_t ()const noexcept{return i;}
        constexpr inline operator size_t &() noexcept{return i;}
    };

    


    template <typename GridType,typename GridContainerType,typename GridIndexType = std::vector<size_t>>
    struct  MultiGrid{
        constexpr static Dim =  1 + InnerGridType::Dim;
        GridType Grid;
        GridContainerType<InnerGridType> InnerGrids;
        GridIndexType Indexes;
        size_t _size;
        size_t size()const{return _size;}
        
        inline MultiGrid(const GridType & Grid,const GridContainerType<InnerGridType>&InnerGrids):Grid(Grid),InnerGrids(InnerGrids){
            initCalcIndex();
        }
        inline MultiGrid(const GridType & Grid,GridContainerType<InnerGridType>&&InnerGrids):Grid(Grid),InnerGrids(std::move(InnerGrids)){
            initCalcIndex();
        }
        inline MultiGrid( GridType && Grid,const GridContainerType<InnerGridType>&InnerGrids):Grid(std::move(Grid)),InnerGrids(InnerGrids){
            initCalcIndex();
        }
        inline MultiGrid( GridType && Grid, GridContainerType<InnerGridType>&&InnerGrids):Grid(std::move(Grid)),InnerGrids(std::move(InnerGrids)){
            initCalcIndex();
        }

        inline MultiIndex<Dim> Multi0()const{return MultiIndex<Dim>();}
        inline MultiIndex<Dim> MultiSize()const{return MultiIndex<Dim>(InnerGrids.size(),MultiIndex<Dim-1>());}

        inline void MultiIncrement(MultiIndex<Dim> & mi){
            InnerGrids[mi.i].MultiIncrement(mi.m);
            if(mi.m == InnerGrids[mi.i].MultiSize()){
                ++i;
                mi.m = 0;
            }
        }

        inline size_t LinearIndex(const MultiIndex<Dim> &mi){
            return Indexes[mi.i] + InnerGrids[mi.i].LinearIndex(mi.m);
        } 

        private:
        inline initCalcIndex(){
            _size = 0;
            size_t _inn_size = InnerGrids.size();
            for(size_t i=0;i<_inn_size;++i)
                Indexes[i] = _size;
                _size += InnerGrids[i].size();
        }
        static Indexes
    };
};

#endif//MULTIDIM_GRID_H