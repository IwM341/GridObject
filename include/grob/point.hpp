#ifndef POINT_HPP
#define POINT_HPP

#include <tuple>
#include "rectangle.hpp"
namespace grob{

    /// @brief namespace containing standart operator << for tuple
    namespace tuple_print{
        namespace __detail{
            template <typename...Args>
            inline void print_tuple_elements_impl(std::ostream & os,Args const&...args);
            
            template <>
            inline void print_tuple_elements_impl(std::ostream & os){}

            
            template <typename Arg>
            inline void print_tuple_elements_impl(std::ostream & os,Arg const & arg){
                os << arg;
            }

            template <typename Arg,typename...Args>
            inline void print_tuple_elements_impl(
                std::ostream & os,Arg const & arg,Args const&...args){
                os << arg << ", ";
                print_tuple_elements_impl(os,args...);
            }
            template <typename TupleType,size_t...I>
            inline void print_tuple(std::ostream & os,TupleType const & _Tp,std::index_sequence<I...>){
                print_tuple_elements_impl(os,std::get<I>(_Tp)...);
            }
        };

        

        

        /// @brief standart operator << for tuple
        template <typename...Args>
        std::ostream & operator << (std::ostream & os,std::tuple<Args...> const & _Tp){
            os << "(";
            __detail::print_tuple(os,_Tp,typename std::make_index_sequence<sizeof...(Args)>{});
            os << ")";
            return os;
        }
    };
    
    template <typename...Types>
    struct Point;
    
    template <>
    struct Point<>: public std::tuple<>{
        typedef  std::tuple<> TBase;
        using TBase::TBase;
    };
    template <typename...Types>
    std::ostream & operator << (std::ostream & os,Point<Types...> const & _Pt){
        using namespace tuple_print;
        os << "Point" << static_cast<typename Point<Types...>::TBase const &>(_Pt);
        return os;

    }

    /// @brief male point such as make_tuple 
    template <typename...Args>
    inline constexpr auto make_point(Args &&...args){
        return Point<typename std::decay<Args>::type...>(std::forward<Args>(args)...);
    }

    namespace __point_detail{
        template <typename _Tp>
        struct PointTuple_t{};

        template <typename...Args>
        struct PointTuple_t<std::tuple<Args...>>{
            typedef Point<Args...> type;
        };
    };
    /// @brief male point from tuple
    template <typename Tuple_t>
    inline constexpr auto make_point_tuple(Tuple_t && _Tp){
        return typename __point_detail::PointTuple_t<std::decay_t<Tuple_t>>::type (std::forward<Tuple_t>(_Tp));
    }

    namespace __point_detail{

        template <size_t tp_size_minus_i>
        struct tuple_reducer{
            template <typename Reducer,typename _Tuple,typename InitValue_t>
            static inline constexpr auto tuple_reduce(Reducer && _R,_Tuple && _Tp,InitValue_t && init){
                return std::forward<Reducer>(_R)(
                        tuple_reducer<tp_size_minus_i-1>::tuple_reduce(
                            std::forward<Reducer>(_R),
                            std::forward<_Tuple>(_Tp),
                            std::forward<InitValue_t>(init)
                        ),
                        std::get<
                                std::tuple_size_v<
                                    std::decay_t<_Tuple>
                                >-tp_size_minus_i
                            >(
                            std::forward<_Tuple>(_Tp)
                        ));
            }
        };
        
        template <>
        struct tuple_reducer<1>{
            template <typename Reducer,typename _Tuple,typename InitValue_t>
            static inline constexpr auto tuple_reduce(Reducer && _R,_Tuple && _Tp,InitValue_t && init){
                return std::forward<Reducer>(_R)(std::forward<InitValue_t>(init),
                        std::get<std::tuple_size_v<std::decay_t<_Tuple>>-1>(
                            std::forward<_Tuple>(_Tp)
                        ));
            }
        };


        template <typename LambdaType,typename PointType,size_t...I>
        inline constexpr auto transform_impl(LambdaType && F,PointType && _Pt,  
            std::index_sequence<I...>){
            return make_point(F(std::get<I>(std::forward<PointType>(_Pt)))...);
        }

        
        template <typename Init_t,size_t...I>
        inline constexpr auto make_lambda_tuple(std::index_sequence<I...> ,Init_t && F){
            return std::make_tuple(std::forward<Init_t>(F)(std::integral_constant<size_t,I>{})...);
        }

        template <typename LambdaType,typename IndexSequence,typename...Points>
        inline constexpr auto multi_transform_impl(LambdaType && F, IndexSequence S,Points&&..._Pts){
            //////////////////////////////////////////   index is value of type std::integral_constant<size_t,I> 
            return make_point_tuple(make_lambda_tuple(S,[&](auto index){
                return std::forward<LambdaType>(F)(std::get<decltype(index)::value>(std::forward<Points>(_Pts))...);
            }));
        }

        template <typename PointType,size_t...I>
        inline constexpr auto rec_unwrap(PointType const & _Pt,
            std::index_sequence<I...>){
            return make_point(_Pt.template rget<I>()...);
        }

        template <typename LambdaType,typename PointType,size_t...I>
        inline constexpr auto rtransform_impl(LambdaType && F,PointType const & _Pt,  
            std::index_sequence<I...>){
            return make_point(F(_Pt.template rget<I>())...);
        }

        template <typename PointType>
        struct rec_dim{
            constexpr static size_t value = 1;
        };

        template <typename...Types>
        struct rec_dim<Point<Types...>>{
            constexpr static size_t value = Point<Types...>::rDim;
        };

        template <>
        struct rec_dim<Point<>>{
            constexpr static size_t value = 0;
        };

        template <typename T>
        struct rec_get_impl{
            template <size_t i,typename U>
            static inline constexpr decltype(auto) get(U && x)noexcept{
                static_assert(i == 0,"error: trying to get non zero member of non tuple type");
                return std::forward<U>(x);
            }
        };

        template <typename...Args>
        struct rec_get_impl<Point<Args...>>{
            template <size_t i,typename PointType>
            static inline constexpr decltype(auto) get(PointType && x)noexcept{
                return x.template rget<i>();
            }
        };
        template <size_t i,typename T>
        inline constexpr decltype(auto) rec_get(T && x){
            return rec_get_impl<typename std::decay<T>::type>::
                template get<i>(std::forward<T>(x));
        }

        template<typename T>
        constexpr bool always_false = false;

        struct nontype_struct{
            template <typename T>
            nontype_struct(T &&){}
        };

        template <bool __has_center>
        struct __get_center{
            template <typename T>
            inline constexpr static auto fun(T const &x) noexcept{return x;}
        };
        
        template <>
        struct __get_center<true>{
            template <typename T>
            inline constexpr static auto fun(T const &x) noexcept{return x.center();}
        };
        template <bool __has_volume>
        struct __get_volume{
            template <typename T>
            inline constexpr static auto fun(T const &x) noexcept{return 1;}
        };
        
        template <>
        struct __get_volume<true>{
            template <typename T>
            inline constexpr static auto fun(T const &x) noexcept{return x.volume();}
        };

        template <typename T>
        constexpr auto has_center_check(T && x) -> decltype(x.center()){
            static_assert(always_false<T>,"has_center_check is not a callable function");
        }
        constexpr auto has_center_check(nontype_struct) -> nontype_struct;

        template <typename T>
        constexpr auto has_volume_check(T && x) -> decltype(x.volume()){
            static_assert(always_false<T>,"has_center_check is not a callable function");
        }
        constexpr auto has_volume_check(nontype_struct) -> nontype_struct;

        template <typename T>
        using has_center = typename std::conditional< std::is_same<
                                                decltype(has_center_check(std::declval<T>())),
                                                nontype_struct
                                            >::value, std::false_type,std::true_type>::type;

        template <typename T>
        using has_volume = typename std::conditional< std::is_same<
                                                decltype(has_volume_check(std::declval<T>())),
                                                nontype_struct
                                            >::value, std::false_type,std::true_type>::type;

        struct get_center{
            template <typename T>
            inline constexpr auto operator() (T const & x)const noexcept{
                return __get_center<has_center<T>::value>::fun(x);
            }
        };
        struct get_volume{
            template <typename T>
            inline constexpr auto operator() (T const & x) const noexcept{
                return __get_volume<has_volume<T>::value>::fun(x);
            }
        };

        template <typename Ret_t,size_t offset,typename Tuple_type,size_t...I>
        Ret_t make_type_from_index_tuple(std::index_sequence<I...>,Tuple_type && _Tp){
            return Ret_t(
                std::forward<decltype(std::get<offset + I>(_Tp))>
                (std::get<offset + I>(_Tp))...
            );
        }
        template <size_t I,typename PointType,typename ArgTupleRefType>
        inline constexpr auto ith_element(ArgTupleRefType && _Tp) noexcept{
            return make_type_from_index_tuple<
                                    typename std::tuple_element<I,typename PointType::TBase>::type,
                                    PointType::template rget_index<I>::value
                                >(
                         std::make_index_sequence<
                            PointType::template rget_item_size<I>::value
                        >{},
                        std::forward<ArgTupleRefType>(_Tp));
        }
        template <typename PointType,typename ArgTupleRefType,size_t...I>
        PointType make_rpoint_impl(std::index_sequence<I...> items,ArgTupleRefType && _Tp){
            return PointType(ith_element<I,PointType>(std::forward<ArgTupleRefType>(_Tp))...
                            );
        }

        template <size_t i,typename PointType>
        struct rget_index_impl;

        template <size_t i,typename Type,typename...Types>
        struct rget_index_impl<i,Point<Type,Types...>>{
            static constexpr size_t value = rec_dim<
                        typename std::tuple_element<i - 1,
                        typename Point<Type,Types...>::TBase>::type
                    >::value + Point<Types...>::template rget_index<i - 1>::value;
        };
        
        template <typename Type,typename...Types>
        struct rget_index_impl<0,Point<Type,Types...>>{
            static constexpr size_t value = 0;
        };

        template <bool i_less_dim,size_t i,typename PointType>
        struct rget_item_impl;

        template <size_t i,typename Type,typename...Types>
        struct rget_item_impl<false,i,Point<Type,Types...>>{
            static constexpr size_t value = 1 + 
                Point<Types...>::template rget_item<
                    i - __point_detail::rec_dim<Type>::value
                >::value;
        };
        
        template <size_t i,typename Type,typename...Types>
        struct rget_item_impl<true,i,Point<Type,Types...>>{
            static constexpr size_t value = 0;
        };

        template < std::size_t... Ns , typename... Ts >
        auto get_std_tuple_tail_impl( std::index_sequence<Ns...> , std::tuple<Ts...> const &t )
        {
            return  std::make_tuple( std::get<Ns+1u>(t)... );
        }
    };

    /// @brief Implementation of Point<Types...> class 
    template <typename Type,typename...Types>
    struct Point<Type,Types...> : public std::tuple<Type,Types...>{
        typedef std::tuple<Type,Types...> TBase;
        using TBase::TBase;

        /// @brief static cast Point<Types...> to tuple<Types...>
        inline constexpr TBase const & as_tuple() const noexcept{
            return *this;
        } 
        /// @brief static cast Point<Types...> to tuple<Types...>
        inline constexpr TBase & as_tuple() noexcept{
            return *this;
        }

        /// @brief Dim is sizeof...Types
        constexpr static size_t Dim = std::tuple_size<TBase>::value;
        /// @brief rDim is recoursive Dim
        /// if a Type in Types... contains Point<Types1...>, than
        /// recoursive Dim sums all Dims in subPoints
        constexpr static size_t rDim = __point_detail::rec_dim<Type>::value + 
                                       __point_detail::rec_dim<Point<Types...>>::value;

        /// index is index of unwrapped Point,
        /// item is wrapped item: Type,Types...
        
        /// @brief rget_item<i>::value gives index of subtype in Point<...> 
        /// which contain recoursive index
        /// @tparam index recoursive index of element in Point,
        /// @example for Point<Point<T0,T1>,T2> index of Ti is i,
        /// and rget_item<0 or 1>::value is 0 and rget_item<2>::value is 1  
        template <size_t index>
        struct rget_item{
            static constexpr size_t value = 
                __point_detail::rget_item_impl<
                    index < __point_detail::rec_dim<Type>::value,
                    index,
                    Point  
                >::value;
        };

        /// @brief rget_index<i>::value gives recoursive start index
        /// of item
        /// @tparam item 
        /// @example for Point<Point<T0,T1>,T2> item of Point<T0, T1> is 0, item of T2 is 1
        /// and rget_index<0,1>::value is 0 and 2   
        template <size_t item>
        struct rget_index{
            static constexpr size_t value = 
            __point_detail::rget_index_impl<
                item,
                Point
            >::value;
        };

        /// @brief rget_item_size<i>::value is rDim of item type
        template <size_t item>
        struct rget_item_size{
            static constexpr size_t value = 
            __point_detail::rec_dim<typename std::tuple_element<item,TBase>::type>::value;
        };

        /// @brief constructor from tuple
        inline constexpr Point(TBase _Tp) noexcept:TBase(std::move(_Tp)){}

        /// @brief head tail constructor 
        inline constexpr Point(Type Head,Point<Types...> Tail) noexcept:
        TBase(std::tuple_cat(std::make_tuple(std::move(Head)),
                   std::move(Tail.as_tuple())) ){}

        /// @brief number of args is rDim, recoursive construcor
        template <typename...Args,
            typename = typename std::enable_if<(sizeof...(Args) > Dim),bool>::type>
        inline constexpr Point(Args&&...args) noexcept: 
            Point(
                    __point_detail::make_rpoint_impl<Point>(
                        std::make_index_sequence<Dim> {},
                        std::forward_as_tuple(args...)
                        )
                ){}



        /// @brief get head = frst element
        template <size_t i>
        inline constexpr auto & head() noexcept{
            return std::get<0>(*this);
        }

        /// @brief get tail
        inline constexpr Point<Types...> tail()const noexcept{
            return __point_detail::get_std_tuple_tail_impl(std::make_index_sequence<Dim-1>{},*this);
        }

        /// @brief equivalent to tuple get
        template <size_t i>
        inline constexpr auto & get() noexcept{
            return std::get<i>(*this);
        }
        /// @brief equivalent to tuple get
        template <size_t i>
        inline constexpr const auto & get() const noexcept{
            return std::get<i>(*this);
        }

        /// @brief equivalent to tuple get
        template <size_t i>
        inline constexpr auto & rget() noexcept{
            constexpr size_t __item =  rget_item<i>::value;
            return __point_detail::
                rec_get<i- rget_index<__item>::value>(std::get<__item>(*this));
        }
        
        template <size_t i>
        inline constexpr const auto & rget() const noexcept{
            constexpr size_t __item =  rget_item<i>::value;
            return __point_detail::
                rec_get<i- rget_index<__item>::value>(std::get<__item>(*this));
        }

        template <size_t i>
        inline constexpr auto & x() noexcept{
            return std::get<i>(*this);
        }
        template <size_t i>
        inline constexpr const auto & x() const noexcept{
            return std::get<i>(*this);
        }

        template <size_t i>
        inline constexpr auto & rx() noexcept{
            return rget();
        }
        template <size_t i>
        inline constexpr const auto & rx() const noexcept{
            return rget();
        }

        inline constexpr auto rpoint()const noexcept{
            return __point_detail::rec_unwrap(*this,std::make_index_sequence<rDim>{});
        }

        template <typename LambdaType>
        inline constexpr auto transform(LambdaType && F)const noexcept{
            return __point_detail::transform_impl(F,*this,std::make_index_sequence<Dim>{});
        }
        template <typename LambdaType>
        inline constexpr auto rtransform(LambdaType && F)const noexcept{
            return __point_detail::rtransform_impl(F,*this,std::make_index_sequence<rDim>{});
        }

        inline constexpr auto center() const noexcept{
            return transform(__point_detail::get_center{});
        }
        inline constexpr auto rcenter() const noexcept{
            return rtransform(__point_detail::get_center{});
        }
        inline constexpr auto volume() const noexcept{
            return __point_detail::tuple_reducer<Dim>::tuple_reduce(
                [](auto init,auto const & V){return init*__point_detail::get_volume{}(V);},
                as_tuple(),
                1
            );
        }

        inline operator Type & (){
            static_assert(sizeof...(Types) == 0, "unpack conversion available for points of one type");
            return get<0>();
        }
        inline operator Type const & () const{
            static_assert(sizeof...(Types) == 0, "unpack conversion available for points of one type");
            return get<0>();
        }
    };

    

    
    /// @brief make point from head and tail
    template <typename TypeHead,typename...TailTypes>
    inline constexpr auto make_point_ht(TypeHead  head, Point<TailTypes...> tail){
        return Point<typename std::decay<TypeHead>::type,TailTypes...>(
            std::move(head), std::move(tail)
        );
    }
    /// @brief make point from head and tail
    template <typename TypeHead, typename TailType>
    inline constexpr auto make_point_ht(TypeHead && head, TailType && tail){
        return Point<typename std::decay<TypeHead>::type,
            typename std::decay<TailType>::type>
            (
            std::forward<TypeHead>(head), std::forward<TailType>(tail)
        );
    }

    template <typename T,typename U>
    struct point_cat{
        typedef Point<T,U> type;
    };
    
    template <typename Type,typename...Types>
    struct point_cat<Type,Point<Types...>>{
        typedef Point<Type,Types...> type;
    };

    template <typename...Args>
    inline constexpr Point<Args...> & 
        as_point(typename Point<Args...>::TBase & _Tp) noexcept{
        return static_cast<Point<Args...> &>(_Tp);
    }
    template <typename...Args>
    inline constexpr const Point<Args...> & 
        as_point(typename  Point<Args...>::TBase const& _Tp)noexcept{
        return static_cast<Point<Args...> const&>(_Tp);
    }

    template <typename...Args1, typename...Args2>
    inline auto intersect(Point<Args1...> const & P1, Point<Args2...> const & P2){
        static_assert(sizeof...(Args1) == sizeof...(Args2), 
            "number of args in Points intersection should be the same");
        auto pre_result =  __point_detail::multi_transform_impl([](auto const &p1,auto const &p2){
                return intersect(p1,p2);
            }, std::make_index_sequence<sizeof...(Args1)>{},P1,P2);
        bool b = __point_detail::tuple_reducer<sizeof...(Args1)>::tuple_reduce([](bool init,auto const & p)->bool{
            return init && p.second;
        },pre_result.as_tuple(),true);
        return std::make_pair(pre_result.transform([](auto const& x){return x.first;}),b);
    }

};

namespace std{
    template <typename...Args>
        struct tuple_size<grob::Point<Args...>>:
            integral_constant<
                size_t,tuple_size<typename grob::Point<Args...>::TBase>::value
            > {};
            
        template< std::size_t I, class...Args >
        struct tuple_element<I,grob::Point<Args...>>:
            std::tuple_element<I,typename grob::Point<Args...>::TBase>{};
    };
#endif //POINT_HPP