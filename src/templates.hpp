#ifndef TEMPLATES_HPP
#define TEMPLATES_HPP
 
#include <type_traits>
#include <tuple>

namespace templdefs{

template<class T, class U>
        struct is_not_same : std::true_type {};
 
        template<class T>
        struct is_not_same<T, T> : std::false_type {};
        
template <typename Type>
struct remove_all{
    typedef typename std::remove_cv<
        typename std::remove_reference<
            typename std::remove_pointer<Type>::type
        >::type
    >::type type;
};

template <typename Base,typename Derived>
struct forward_qualifiers{
    typedef Derived type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<const Base,Derived>{
    typedef const Derived type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<Base &,Derived>{
    typedef Derived & type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<volatile Base &,Derived>{
    typedef volatile Derived & type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<const Base &,Derived>{
    typedef Derived const& type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<volatile const Base &,Derived>{
    typedef volatile Derived const& type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<Base *,Derived>{
    typedef Derived * type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<volatile Base *,Derived>{
    typedef volatile Derived * type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<const Base *,Derived>{
    typedef Derived const* type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<volatile const Base*,Derived>{
    typedef volatile Derived const* type;
};

template <typename Base,typename Derived>
struct forward_qualifiers<Base &&,Derived>{
    typedef Derived && type;
};


template <typename T,typename U>
struct tuple_cat{
    typedef std::tuple<T,U> type;
};

template <typename T,typename...Args>
struct tuple_cat<T,std::tuple<Args...>>{
    typedef std::tuple<T,Args...> type;
};

template <size_t tuple_size>
struct tuple_reducer{
    template <typename Tuple>
    static decltype(auto) reduce(Tuple && T){
        return std::forward<Tuple>(T);
    }
};
template <>
struct tuple_reducer<1>{
    template <typename Tuple>
    static decltype(auto) reduce(Tuple && T){
        return std::get<0>(std::forward<Tuple>(T));
    }
};

template <typename Tuple>
decltype(auto) tuple_reduce(Tuple && X){
    return tuple_reducer<std::tuple_size<typename std::decay<Tuple>::type>::value>::reduce(std::forward<Tuple>(X));
};

template <class F, class Tuple>
constexpr decltype(auto) apply_tuple(F&& f, Tuple&& t);

namespace apply_detail {
template <class F, class Tuple, std::size_t... I>
inline constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
{
  return f(std::get<I>(std::forward<Tuple>(t))...);
  // Note: std::invoke is a C++17 feature
}
}; // namespace detail
 
template <class F, class Tuple>
constexpr decltype(auto) apply_tuple(F&& f, Tuple&& t)
{
    return apply_detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{});
}

};
#endif//TEMPLATES_HPP