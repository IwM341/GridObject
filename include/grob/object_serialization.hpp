#ifndef OBJECT_SERIALIZATION_HPP
#define OBJECT_SERIALIZATION_HPP

#include <type_traits>
#include <array>
#include <vector>
#include "templates.hpp"

#define OBJECT_DESERIALIZATION_FUNCTION(ClassName) \
    template <typename ObjecType,typename DeSerializer>\
        static ClassName DeSerialize(ObjecType && Object,DeSerializer && S){\
            ClassName Ret;\
            Ret.init_serialize(std::forward<ObjecType>(Object),std::forward<DeSerializer>(S));\
            return Ret;\
        }

#define OBJECT_READ_FUNCTION(ClassName) \
    template <typename ReaderStreamType>\
        static ClassName read(ReaderStreamType && r){\
            ClassName Ret;\
            Ret.init_read(std::forward<ReaderStreamType>(r));\
            return Ret;\
        }

#define INHERIT_SERIALIZATOR(Base) \
    template <typename Serializer>\
    inline auto Serialize(Serializer && S)const{\
        return stools::Serialize(static_cast<Base const &>(*this),S);\
    }\
    template <typename Writer>\
    inline void write(Writer && w){\
        stools::write(static_cast<Base const &>(*this),w);\
    }    
#define INHERIT_DESERIALIZATOR(Base,Derived) \
    template <typename Object,typename DeSerializer>\
    static Derived DeSerialize(Object const& Obj,DeSerializer && DS){\
        return (Base::DeSerialize(Obj,std::forward<DeSerializer>(DS)));\
    }\
    template <typename Reader>\
    static Derived read(Reader && r){\
        return (Base::read(r));\
    }





/*!
    Object Serialization:
    Serializator shuld be a type with functions (static or not):
    1) MakePrimitive(template T x) -> Object, wich makes some object from primitive type
    2) MakeDict(size,template LambdaNames : size_t -> string,template LambdaValues : size_t -> Object) -> Object
    3) MakeArray(size,template LambdaValues : size_t -> Object) -> Object
    DeSerializator shoud have functions:
    1) GetProperty(Object,Name : string) -> Object
    2) GetPrimitive(Object,template T & result)
    ~3) GetSize(Object) -> size_t size of array if Object is array  optional
    ~4) GetItem(Object,size_t) -> Object 
    3) Begin(Object) --- returns iterator it, that *is is Object (array) and pair<string,Object> - dict  
    4) End(Object) --- end iterator
    5) GetKey(iterator)
    6) GetValue(iterator)
*/
namespace stools{

    template <typename T,typename Serializer>
        auto Serialize(T const& x,Serializer && S);
    template <typename DeSerializer,typename Object,typename T,size_t N>
        void init_serialize(std::array<T,N> & V,Object && Obj,DeSerializer && DS);

    template <typename T,typename DeSerializer,typename Object>
        auto DeSerialize(Object && Obj,DeSerializer && DS);

    template <typename T,typename Writer>
        void write(T const & x, Writer && w);

    template <typename T,typename Reader>
        void init_read(T & x, Reader && w);

    template <typename T,typename Reader>
        auto read(Reader && r);

    namespace _serialize_impl{
        struct not_serializable{};

        template <class C,typename Serializer>
        auto serializable_check(C const& c,Serializer && S)->decltype(c.Serialize(S));

        not_serializable serializable_check(...);

        

        template <class C,typename Serializer>
        struct is_serializable : templdefs::is_not_same<not_serializable,
                    decltype(
                        serializable_check(std::declval<C>(),std::declval<Serializer>())
                    )>{};

        template <bool _is_srlz>
        struct _condition_serialize{
            template <class C,typename Serializer>
            static auto function(C const& c,Serializer && S){
                return S.MakePrimitive(c);
            }
        };
        template <>
        struct _condition_serialize<true>{
            template <class C,typename Serializer>
            static auto function(C const& c,Serializer && S){
                return c.Serialize(S);
            }        
        };




        struct not_deserializable{};

        template <class C,typename Object,typename DeSerializer>
        auto deserializable_check(C && c,Object && Obj,DeSerializer && DS)->decltype(c.init_serialize(Obj,DS));

        not_deserializable deserializable_check(...);

        template <class C,typename Object,typename Serializer>
        struct is_deserializable : templdefs::is_not_same<not_deserializable,
                    decltype(
                        deserializable_check(std::declval<C>(),std::declval<Object>(),std::declval<Serializer>())
                    )>{};

        template <bool _is_srlz>
        struct _condition_init_deserialize{
        template<class C,typename Object,typename Serializer>
            static auto function(C & c,Object && Obj,Serializer && S){
                return S.GetPrimitive(Obj,c);
            }
        };
        template <>
        struct _condition_init_deserialize<true>{
            template <class C,typename Object,typename Serializer>
            static auto function(C && c,Object && Obj,Serializer && S){
                return c.init_serialize(Obj,S);
            }
        };


        struct not_simply_deserializable{};
        template <class C,typename Object,typename DeSerializer>
        auto simple_deserializable_check(C && c,Object && Obj,DeSerializer && DS)->decltype(std::decay<C>::type::DeSerialize(Obj,DS));
        not_simply_deserializable simple_deserializable_check(...);

        template <class C,typename Object,typename Serializer>
        struct is_simply_deserializable : templdefs::is_not_same<not_simply_deserializable,
                    decltype(
                        simple_deserializable_check(std::declval<C>(),std::declval<Object>(),std::declval<Serializer>())
                    )>{};

        template <bool _is_srlz>
        struct _condition_deserialize {
            template <class C,typename Object,typename Serializer>
            static auto function(Object && Obj,Serializer && S){
                C c;
                init_serialize(c,Obj,S);
                return c;
            }
        };
        template<>
        struct  _condition_deserialize<true>{ 
            template <class C,typename Object,typename Serializer>
            static auto function(Object && Obj,Serializer && S){
                return C::DeSerialize(Obj,S);
            }
        };
    };
    
    template <typename T,typename Serializer>
    auto Serialize(T const& x,Serializer && S){
        return _serialize_impl::_condition_serialize<
                    _serialize_impl::is_serializable<T,Serializer>::value
                >::function(x,S);
    }

    template <typename...Args,typename Serializer>
    auto Serialize(std::vector<Args...> const& x,Serializer && S){
        return S.MakeArray(x.size(),[&x,&S](size_t i){
            return Serialize(x[i],S);
        });
    }

    template <typename T,size_t N,typename Serializer>
    auto Serialize(std::array<T,N> const& x,Serializer && S){
        return S.MakeArray(N,[&x,&S](size_t i){
            return Serialize(x[i],S);
        });
    }

    template <typename Result,typename Object,typename DeSerializer>
    void init_serialize(Result & R,Object && Obj,DeSerializer && DS){
        _serialize_impl::_condition_init_deserialize<
                    _serialize_impl::is_deserializable<Result,Object,DeSerializer>::value
                >::function(R,Obj,DS);
    }

    

    template <typename T>
    struct _deserialize_helper{
        template <typename DeSerializer,typename Object>
        static typename std::decay<T>::type DeSerialize(Object && Obj,DeSerializer && DS){
            return _serialize_impl::_condition_deserialize<
                    _serialize_impl::is_simply_deserializable<T,Object,DeSerializer>::value
                >::template function<typename std::decay<T>::type>(Obj,DS);
        }
    };
    template <typename T,typename...Args>
    struct _deserialize_helper<std::vector<T,Args...>>{
        typedef std::vector<T,Args...> Container;
        template <typename DeSerializer,typename Object>
        static inline auto _condition_reserve_vector(Container & V,Object && Obj,DeSerializer && DS)->decltype(DS.GetSize(Obj)){
            size_t size = DS.GetSize(Obj);
            V.reserve(size);
            return size;
        }
        struct convertible_from_any{
            template <typename AnyType>
            convertible_from_any(AnyType && ){}
        };
        static inline size_t _condition_reserve_vector(convertible_from_any,convertible_from_any,convertible_from_any) noexcept{
            return 0;
        }


        template <typename DeSerializer,typename Object>
        static auto DeSerialize(Object && Obj,DeSerializer && DS){
            Container V;
            _condition_reserve_vector((Container &)V,Obj,DS);
            for(auto it = DS.Begin(Obj);it != DS.End(Obj);++it){
                V.push_back(_deserialize_helper<typename std::decay<T>::type>::DeSerialize(DS.GetValue(it),DS));
            }
            return V;
        }
        template <typename DeSerializer,typename Object>
        static void init_serialize(std::vector<Args...> & V,Object && Obj,DeSerializer && DS){
            return V = DeSerialize(Obj,DS);
        }
    };
    template <typename T,size_t N>
    struct _deserialize_helper<std::array<T,N>>{
        template <typename DeSerializer,typename Object>
        static auto DeSerialize(Object && Obj,DeSerializer && DS){
            std::array<T,N> V;
            size_t i;
            for(auto it = DS.Begin(Obj),i=0;it != DS.End(Obj);++it,++i){
                V[i] = _deserialize_helper<typename std::decay<T>::type>::DeSerialize(DS.GetValue(it),DS);
            }
            return V;
        }
    };
    template <typename DeSerializer,typename Object,typename T,typename...Args>
    void init_serialize(std::vector<T,Args...> & V,Object && Obj,DeSerializer && DS){
        _deserialize_helper<std::vector<T,Args...>>::_condition_reserve_vector(V,Obj,DS);
        //write_json(std::cout,Obj);
        for(auto it = DS.Begin(Obj);it != DS.End(Obj);++it){
            V.push_back(_deserialize_helper<typename std::decay<T>::type>::DeSerialize(DS.GetValue(it),DS));
        }
    }
    /*
    namespace boost::property_tree{
        struct ptree;
        void write_json(std::ostream & os,boost::property_tree::ptree const& p);
    };*/
    template <typename DeSerializer,typename Object,typename T,size_t N>
    void init_serialize(std::array<T,N> & V,Object && Obj,DeSerializer && DS){
        //write_json(std::cout,Obj);
        for(auto it = DS.Begin(Obj),i=0;it != DS.End(Obj);++it,++i){
            V[i] = _deserialize_helper<typename std::decay<T>::type>::DeSerialize(DS.GetValue(it),DS);
        }
    }

    template <typename T,typename DeSerializer,typename Object>
    auto DeSerialize(Object && Obj,DeSerializer && DS){
        return _deserialize_helper<typename std::decay<T>::type>::DeSerialize(Obj,DS);
    }

    
    namespace _read_write_impl{
        struct not_writable{};
        struct not_init_readable{};
        struct not_readable{};

        template <typename T,typename Writer>
        auto write_check(T && x,Writer && w)->decltype(x.write(w));
        not_writable write_check(...);

        template <typename T,typename Writer>
        struct is_writable: public templdefs::is_not_same<not_writable,
            decltype(write_check(std::declval<T>(),std::declval<Writer>()))
            >{};

        template <bool _writable>
        struct _condition_write{
            template <typename T,typename Writer>
            static void function(T && x,Writer && w){
                w.write(x);
            }
        };

        template<>
        struct  _condition_write<true>{
            template <typename T,typename Writer>
            static void function(T && x,Writer && w){
                x.write(w);
            };
        };

        template <typename T,typename Reader>
        auto init_read_check(T && x,Reader && r)->decltype(x.init_read(r));
        not_init_readable init_read_check(...);

        template <typename T,typename Reader>
        struct is_init_readable: public templdefs::is_not_same<not_init_readable,
            decltype(init_read_check(std::declval<T>(),std::declval<Reader>()))
            >{};

        template <bool init_readable>
        struct _condition_init_read{
            template <typename T,typename Reader>
            void function(T && x,Reader && r){
                r.read(x);
            }
        };
        template <>
        struct _condition_init_read<true>{
            template <typename T,typename Reader>
            void function(T && x,Reader && r){
                x.init_read(r);
            }
        };

        template <typename T,typename Reader>
        auto read_check(T && x,Reader && r)->decltype(typename std::decay<T>::type::read(r));
        not_readable read_check(...);

        template <typename T,typename Reader>
        struct is_readable: public templdefs::is_not_same<not_readable,
            decltype(read_check(std::declval<T>(),std::declval<Reader>()))
            >{};
        
        template <bool _readable>
        struct _condition_read{
            template <typename T,typename Reader>
            auto function(Reader && r){
                T x;
                init_read(x,r);
                return x;
            }
        };

        template <>
        struct _condition_read<true>{
            template<typename T,typename Reader>
            auto function(Reader && r){
                return typename std::decay<T>::type::read(r);
            }
        };
    };

    template <typename T,typename Writer>
    void write(T const & x, Writer && w){
        return _read_write_impl::_condition_write<
                _read_write_impl::is_writable<T,Writer>::value
            >::function(x,w);
    }

    template <typename...Args,typename Writer>
    void write(std::vector<Args...> const & x, Writer && w){
        w.write(x.size());
        for(auto const & v : x){
            write(v,w);
        }
    }
    template <typename T,size_t size,typename Writer>
    void write(std::array<T, size> const & x, Writer && w){
        w.write(x.size());
        for(auto const & v : x){
            write(v,w);
        }
    }
    /****/
    template <typename T,typename Reader>
    void init_read(T & x, Reader && w){
        return _read_write_impl::_condition_init_read<
                _read_write_impl::is_init_readable<T,Reader>::value
            >::function(x,w);
    }

    template <typename...Args,typename Reader>
    void init_read(std::vector<Args...> & x, Reader && r){
        size_t size;
        r.read(size);
        x.resize(size);
        for(size_t i=0;i<size;++i){
            init_read(x[i],r);
        }
    }
    
    template <typename T,size_t size,typename Reader>
    void init_read(std::array<T,size> & x, Reader && r){
        size_t _size;
        r.read(_size);
        if(size != _size){
            throw std::range_error("error in static GridArray read(ReaderStreamType && r)");
        }
        for(size_t i=0;i<size;++i){
            init_read(x[i],r);
        }
    }


    template <typename T>
    struct _read_helper{
        template <typename Reader>
        auto read_(Reader && r){
            return _read_write_impl::_condition_read<
                    _read_write_impl::is_readable<T,Reader>::value
                >::template function<T>(r);
        }
    };
    template <typename T,typename...Args>
    struct _read_helper<std::vector<T,Args...>>{
        template <typename Reader>
        auto read_(Reader && r){
            size_t _size;
            r.read(_size);
            std::vector<T,Args...> V;
            V.reserve(_size);
            for(size_t i=0;i<V.size();++i){
                V.push_back(_read_helper<T>::read_(r));
            }
        }
    };

    template <typename T,typename Reader>
    void read(Reader && r){
        return _read_write_impl::_condition_read<
                _read_write_impl::is_readable<T,Reader>::value
            >::function(r);
    }


    namespace print_vector{
        template <typename...Args>
        std::ostream & operator <<(std::ostream & os, std::vector<Args...> const & V){
            std::ostringstream S;
            //V = "bnmk";
            S << "Vector[";
            for(size_t i=0;i<V.size();++i){
                if(i){
                    S <<", " << V[i];
                }
                else{
                    S << V[i];
                }
            }
            S << "]";
            return os << S.str();
        }
        template <typename T,  size_t N>
        std::ostream & operator <<(std::ostream & os, std::array<T,N> const & A){
            std::ostringstream S;
            S << "Array[";
            for(size_t i=0;i<N;++i){
                if(i){
                    S <<", " << A[i];
                }
                else{
                    S << A[i];
                }
            }
            S << "]";
            return os << S.str();
        }

        template <typename T>
        inline void __print__including__vectors__(std::ostream & os,T const & value){
            os << value;
        }
    };

};

namespace st_detail{
    template <typename Arg,typename ...Args>
    struct first_type{
        typedef Arg type;
    };

    template <typename Translatora_t,typename ...Args>
    inline auto make_array(Translatora_t && tr,Args const&...args){
        return std::array<
                    decltype(tr(std::declval<typename first_type<Args...>::type>())),
                    templdefs::arg_num<Args...>::value
                > {tr(args)...};
    }
    template <typename Lambda_t,typename Arg,typename...Args>
    inline void apply(Lambda_t && Lambda,Arg && arg,Args && ...args){
        Lambda(arg);
        apply(Lambda,args...);
    }
    template <typename Lambda_t>
    inline void apply(Lambda_t && Lambda){}

    template <typename IndexType>
    struct meta_deserialization_impl;

    template <typename IndexType>
    struct meta_read_impl;

    template <size_t...I>
    struct meta_deserialization_impl<std::index_sequence<I...>>{
        template <typename TupleType,typename ConstructorLambda,typename Object,typename DeDerializer,typename NameArray_t >
        static auto construct(ConstructorLambda && Constructor,Object && Obj,DeDerializer && DS,NameArray_t const & names){
            return Constructor(
                stools::DeSerialize<std::tuple_element<I, TupleType>::type>
                    (DS.GetProperty(Obj,names[I]),DS)...
                );
        }
    };
    template <size_t...I>
    struct meta_read_impl<std::index_sequence<I...>>{
        template <typename TupleType,typename ConstructorLambda,typename  Reader>
        static auto construct(ConstructorLambda && Constructor,Reader && r){
            return Constructor(
                stools::read<std::tuple_element<I, TupleType>::type>(r)...
                );
        }
    };
    template <typename TupleType,typename ConstructorLambda,typename Object,typename DeDerializer,typename NameArray_t>
    auto meta_deserialization(ConstructorLambda && Constructor,Object && Obj,DeDerializer && DS,NameArray_t const & names){
        return meta_deserialization_impl<
                    std::make_index_sequence<
                        std::tuple_size<typename std::decay<TupleType>::type>::value
                    >
                >::construct(std::forward<ConstructorLambda>(Constructor),Obj,DS,names);

    }
    template <typename TupleType,typename ConstructorLambda,typename  Reader>
    auto meta_read(ConstructorLambda && Constructor,Reader && r){
        return meta_read_impl<
                    std::make_index_sequence<
                        std::tuple_size<typename std::decay<TupleType>::type>::value
                    >
                >::construct(std::forward<ConstructorLambda>(Constructor),r);
    }


};


//#define ARG_NAMES(...) std::array<char *, std::tuple_size<decltype(std::make_tuple(#__VA_ARGS__))>> {__VA_ARGS__}

#define SERIALIZATOR_FUNCTION(NAMES_ARRAY,PROPERTIES_ARRAY)\
    template <typename Serializer>\
    auto Serialize(Serializer && S)const{\
        static const auto names = NAMES_ARRAY;\
        return S.MakeDict(names,PROPERTIES_ARRAY);\
    }

#define WRITE_FUNCTION(...)\
    template <typename Writer>\
    auto write(Writer && W)const{\
        st_detail::apply([&W](auto const & value){W.write(value);},__VA_ARGS__);\
    }

#define DESERIALIZATOR_FUNCTION(CONSTRUCTOR,NAMES_ARRAY,TYPES_ARRAY)\
    template <typename Object,typename Serializer>\
    static auto DeSerialize(Object && Obj,Serializer && S){\
        return st_detail::meta_deserialization<TYPES_ARRAY>([&](auto &&...args)\
            {\
                return CONSTRUCTOR(std::forward<decltype(args)...>(args...));\
            },Obj,S,NAMES_ARRAY\
        );\
    }

#define READ_FUNCTION(CONSTRUCTOR,TYPES_ARRAY)\
    template <typename Reade>\
    static auto read(Reade && r){\
        return st_detail::meta_read<TYPES_ARRAY>([&](auto &&...args)\
            {\
                return CONSTRUCTOR(std::forward<decltype(args)...>(args...));\
            },r\
        );\
    }

#define PROPERTY_NAMES(...) std::array<std::string,ARG_COUNT(__VA_ARGS__)> {__VA_ARGS__}
#define PROPERTIES(...) st_detail::make_array([&S](const auto & value){return stools::Serialize(value,S);},__VA_ARGS__)
#define PROPERTY_TYPES(...) decltype(std::make_tuple(__VA_ARGS__))

#endif
