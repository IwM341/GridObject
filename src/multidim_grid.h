#ifndef MULTIDIM_GRID_H
#define MULTIDIM_GRID_H
#include "grid.h"

namespace grid_object{
    

    template <size_t N>
    struct MultiIndex{
        size_t i;
        MultiIndex<N-1> m;
        
        inline MultiIndex(size_t j=0):i(0),m(j){}
        inline MultiIndex& operator = (size_t j){
            i=0;
            m = j;
        }
        inline bool operator !=(const MultiIndex& mi ){
            return (i!=m || m!= mi.m);
        }
        inline bool operator ==(const MultiIndex& mi ){
            return (i==m && m== mi.m);
        }
        inline bool operator <(const MultiIndex& mi ){
            return (i<mi ? true : (i > mi ? false : m<mi));
        }
        inline bool operator <=(const MultiIndex& mi ){
            return (i<mi ? true : (i > mi ? false : m<=mi));
        }
        friend inline operator >(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<mi_1;
        }
        friend inline operator >=(const MultiIndex& mi_1,const MultiIndex& mi_2){
            return mi_2<=mi_1;
        }
        template <typename...Inners>
        inline MultiIndex(size_t i,Inners...inn):i(0),m(inn...){}
    };

    template <>
    struct MultiIndex<1>{
        size_t i;
        inline MultiIndex<1>(size_t i=0):i(i){}
        inline operator size_t ()const{return i;}
        inline operator size_t &(){return i;}
    };

    


    template <typename GridType,typename GridContainerType,typename GridIndexType = std::vector<size_t>>
    struct  MultiGrid{
        constexpr static Dim =  1 + InnerGridType::Dim;
        GridType Grid;
        GridContainerType<InnerGridType> InnerGrids;
        GridIndexType Indexes;
        size_t _size;
        size_t size()const{return _size;}
        
        inline MultiGrid(cosnt GridType & Grid,const GridContainerType<InnerGridType>&InnerGrids):Grid(Grid),InnerGrids(InnerGrids){
            initCalcIndex();
        }
        inline MultiGrid(cosnt GridType & Grid,GridContainerType<InnerGridType>&&InnerGrids):Grid(Grid),InnerGrids(std::move(InnerGrids)){
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