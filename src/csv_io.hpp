#ifndef CSV_IO_HPP
#define CSV_IO_HPP

#include "grid_objects.hpp"
#include <array>
#include <tuple>
#include <iostream>
#include <type_traits>

namespace grob{

namespace detail{
    
    template <typename T,size_t tp_size,size_t pos = 0>
    struct dynamic_tuple_getter{
        template <typename Tuple>
        static T get(Tuple const & _Tp,size_t index) noexcept{
            if(index == pos){
                return std::get<pos>(_Tp);
            }
            else 
                return dynamic_tuple_getter<T,tp_size,pos+1>::get(_Tp,index);
        }
    };

    template <typename T,size_t tp_size>
    struct dynamic_tuple_getter<T,tp_size,tp_size>{
        template <typename Tuple>
        static T get(Tuple const & _Tp,size_t index) noexcept{
            return std::get<tp_size-1>(_Tp);
        }
    };


    template <size_t tp_size,size_t pos = 0>
    struct tp_to_csv{
        template <typename stream_type,typename Tuple>
        static inline void write(stream_type & os,Tuple const &_Tp,char delim){
            os << std::get<pos>(_Tp) << delim; 
            tp_to_csv<tp_size,pos+1>::write(os,_Tp, delim);
        }
    };
    template <size_t tp_size>
    struct tp_to_csv<tp_size,tp_size>{
        template <typename stream_type,typename Tuple>
        static inline void write(stream_type & os,Tuple const &_Tp,char delim){}
    };

    template <>
    struct tp_to_csv<1,0>{
        template <typename stream_type,typename NotTuple>
        static inline void write(stream_type & os,NotTuple const &_Tp,char delim){
            os << _Tp << delim; 
        }
        template <typename stream_type,typename...Args>
        static inline void write(stream_type & os,std::tuple<Args...> const &_Tp,char delim){
            os << std::get<0>(_Tp) << delim; 
        }
    };   

};

template <typename GridObjectType>
struct csv_viewer{
    typedef typename std::decay<GridObjectType>::type  GOType;
    GOType const & Object;

    csv_viewer(GridObjectType const & Object):Object(Object){}
    size_t rows()const noexcept{return Object.Grid.size();}
    size_t columns()const noexcept{return GridObjectType::Dim + 1;}
    double operator ()(size_t i,size_t j){
        if(j == GridObjectType::Dim )
            return  Object.Values[j];
        else{
            auto MI = Object.Grid.LinearIndex(i);
            return detail::dynamic_tuple_getter<double, GOType::Dim>::get( Object.Grid[MI]);
        }
    }
    template <typename stream_type>
    void save(stream_type && os)const{
        size_t i=0;
        for(auto MI = Object.Grid.Multi0();MI != Object.Grid.MultiSize();Object.Grid.MultiIncrement(MI),++i){
            detail::tp_to_csv<GOType::Dim>::write(os,Object.Grid[MI],'\t');
            os << Object.Values[Object.Grid.LinearIndex(MI)] << std::endl;
        }
    }
};

template <typename GridObjectType>
csv_viewer<GridObjectType> as_csv(GridObjectType const & Obj){
    return Obj;
};

};
#endif//CSV_IO_HPP