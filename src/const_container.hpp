#ifndef CONST_CONTAINER_HPP
#define CONST_CONTAINER_HPP

#include <memory>
#include <iostream>
#include <sstream>
#include "object_serialization.hpp"

namespace grob{
    
    template <typename T>
    class ConstValueVector{
        
        T value;
        size_t size_;

        public:
        typedef T value_type;

        inline constexpr size_t size() const noexcept{
            return size_;
        }
        inline constexpr T const& operator[](size_t) const noexcept{
            return value;
        }
        inline constexpr T & operator[](size_t)  noexcept{
            return value;
        }

        ConstValueVector(size_t size_ = 0):size_(size_){}
        inline constexpr ConstValueVector(T value,size_t size_ = 0) noexcept:value(std::forward<T>(value)),size_(size_) {}
        
        inline void resize(size_t new_size)noexcept{size_ = new_size;}

        inline constexpr T & first() noexcept{return value;}
        inline constexpr T const& first() const noexcept{return value;}

        inline constexpr T & back() noexcept{return value;}
        inline constexpr T const& back() const noexcept{return value;}


        friend std::ostream & operator << (std::ostream & os,ConstValueVector const&V){
            std::ostringstream S;
            S << "ConstValueVector[" << V.value << "]";
            return os << S.str();
        }

        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(2,
                [](size_t i){ return (!i ? "size":"value");},
                [this,&S](size_t i){
                    return ( !i? stools::Serialize(size_,S) : 
                        stools::Serialize(value,S));
                    }
            );
        }

        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            stools::init_serialize(size_,S.GetProperty(Object,"size"),S);
            stools::init_serialize(value,S.GetProperty(Object,"value"),S);
        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w){
            stools::write(size_,w);
            stools::write(value,w);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            stools::init_read(size_,r);
            stools::init_read(value,r);
        }
        OBJECT_DESERIALIZATION_FUNCTION(ConstValueVector)
        OBJECT_READ_FUNCTION(ConstValueVector)
    };

    template <typename T>
    struct ConstReferenceVector:public ConstValueVector<T &>{
        using ConstValueVector<T &>::ConstValueVector;
        typedef T value_type;
    };

    template <typename T>
    class ConstSharedValueVector{
        
        std::shared_ptr<T> value_ptr;
        size_t size_;

        public:
        typedef T value_type;
        inline auto & getPtr()noexcept {return value_ptr;}
        inline const auto& getPtr()const noexcept {return value_ptr;}
        inline constexpr size_t size() const noexcept{
            return size_;
        }
        inline constexpr T const& operator[](size_t) const noexcept{
            return *value_ptr;
        }
        inline constexpr T & operator[](size_t)  noexcept{
            return *value_ptr;
        }

        inline constexpr ConstSharedValueVector(size_t size_ = 0,  std::shared_ptr<T>  value_ptr = nullptr) noexcept:value_ptr(value_ptr),size_(size_) {}

        inline constexpr ConstSharedValueVector(size_t size_,T value) noexcept:value_ptr(std::make_shared<T>(std::forward<T>(value))),size_(size_) {}
        
        inline void resize(size_t new_size)noexcept{size_ = new_size;}

        inline constexpr T & first() noexcept{return *value_ptr;}
        inline constexpr T const& first() const noexcept{return *value_ptr;}

        inline constexpr T & back() noexcept{return *value_ptr;}
        inline constexpr T const& back() const noexcept{return *value_ptr;}

        friend std::ostream & operator << (std::ostream & os,ConstSharedValueVector const&V){
            std::ostringstream S;
            S << "ConstValueVector[" << *V.value_ptr << "]";
            return os << S.str();
        }


        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(2,
                [](size_t i){ return (!i ? "size":"value");},
                [this,&S](size_t i){
                    return ( !i? stools::Serialize(size_,S) : 
                        stools::Serialize(*value_ptr,S));
                    }
            );
        }

        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            init_serialize(size_,S.GetProperty(Object,"size"));
            if(value_ptr != nullptr)
                init_serialize(*value_ptr,S.GetProperty(Object,"value"));
            else 
                value_ptr = std::shared_ptr<T>(stools::DeSerialize<T>(Object,S));

        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w)const{
            write(size_,w);
            write(*value_ptr,w);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            init_read(size_,r);
            init_read(*value_ptr,r);
        }

        template <typename ObjecType,typename DeSerializer>
        static ConstSharedValueVector DeSerialize(ObjecType && Object,DeSerializer && S){
            size_t _size;
            S.GetPrimitive(S.GetProperty(Object,"size"),_size);
            return ConstSharedValueVector (_size,stools::DeSerialize<T>(S.GetProperty(Object,"value"),S));
        }
        template <typename ReaderStreamType>
        ConstSharedValueVector read(ReaderStreamType && r){
            size_t _size;
            init_read(_size,r);
            return ConstSharedValueVector (_size,stools::read<T>(r));
        }
            

    };

    struct LinearIndexer{
        size_t size_;
        size_t len_;
        
        LinearIndexer(size_t size_ = 0,size_t len_ = 1):size_(size_),len_(len_){}
        size_t inline constexpr  operator [](size_t i)const noexcept{return i*len_;}
        size_t inline constexpr  size()const noexcept{return size_;}

        inline constexpr size_t first() noexcept{return 0;}
        inline constexpr size_t back() noexcept{return len_*(size_-1);}
    };

    template <typename Container>
    struct is_const_container{
        constexpr static bool value = false;
    };

    template <typename T>
    struct is_const_container<ConstSharedValueVector<T>>{
        constexpr static bool value = true;
    };

    
    template <typename T>
    struct is_const_container<ConstValueVector<T>>{
        constexpr static bool value = true;
    };

    template <typename T>
    struct is_const_container<ConstReferenceVector<T>>{
        constexpr static bool value = true;
    };


    template <typename FuncType>
    constexpr auto make_vector(size_t size,FuncType && F){
        std::vector<typename std::decay<decltype(F(0))>::type > V;
        V.reserve(size);
        for(size_t i=0;i<size;++i){
            V.push_back(F(i));
        }
        return V;
    } 

    namespace _index_impl{
        template <typename ContainerOfContainer,typename Iterator>
        inline void _init_index_count(const ContainerOfContainer & _CC,Iterator it) noexcept{
            *it = 0;
            size_t count = 0;
            for(size_t i=0;i<_CC.size();++i){
                count += _CC[i].size();
                *(++it) = count;
            }
        }

        template <typename ContainerOfContainer>
        struct _index_container{
            typedef std::vector<size_t> type;
            constexpr static bool is_noexcept = false;

            template <typename ContainerOfContainer_in>
            static type Create(const ContainerOfContainer_in & V){
                type Vret(V.size()+1);
                _index_impl::_init_index_count(V,Vret.begin());
                return Vret;
            }

            template <typename ContainerOfContainer_in>
            constexpr static void Update(const ContainerOfContainer_in & _CC,type & Indexes)noexcept{
                _index_impl::_init_index_count(_CC,Indexes.begin());
            }
        };

        template <typename ContainerType>
        struct _index_container<ConstValueVector<ContainerType>>{
            typedef LinearIndexer type;
            constexpr static bool is_noexcept = true;
            template <typename ContainerOfContainer_in>
            static type Create(const ContainerOfContainer_in & V) noexcept{
                return type(V.size()+1, (V.size() ? V[0].size() : 0));
            }

            template <typename ContainerOfContainer_in>
            constexpr static void Update(const ContainerOfContainer_in & V,type & Indexes)noexcept{
                Indexes = Create(V);
            }
        };

        template <typename ContainerType>
        struct _index_container<ConstSharedValueVector<ContainerType>>{
            typedef LinearIndexer type;

            constexpr static bool is_noexcept = true;
            template <typename ContainerOfContainer_in>
            static type Create(const ContainerOfContainer_in & V) noexcept{
                return type(V.size()+1, (V.size() ? V[0].size() : 0));
            }

            template <typename ContainerOfContainer_in>
            constexpr static void Update(const ContainerOfContainer_in & V,type & Indexes)noexcept{
                Indexes = Create(V);
            }
        };
        template <typename ContainerType,typename...VArgs>
        struct _index_container<std::vector<ContainerType,VArgs...>>{
            typedef std::vector<size_t,VArgs...> type;
            constexpr static bool is_noexcept = false;

            template <typename ContainerOfContainer_in>
            constexpr static type Create(const ContainerOfContainer_in & _CC){
                type Vret(_CC.size()+1);
                _index_impl::_init_index_count(_CC,Vret.begin());
                return Vret;
            }
            template <typename ContainerOfContainer_in>
            constexpr static void Update(const ContainerOfContainer_in  & _CC,type & Indexes)noexcept{
                _index_impl::_init_index_count(_CC,Indexes.begin());
            }
        };

        template <typename ContainerType,size_t _size>
        struct _index_container<std::array<ContainerType,_size>>{
            typedef std::array<size_t,_size> type;
            constexpr static bool is_noexcept = true;
            template <typename ContainerOfContainer_in>
            constexpr static type Create(const ContainerOfContainer_in & _CC)noexcept{
                type Vret;
                _index_impl::_init_index_count(_CC,Vret.begin());
                return Vret;
            }

            template <typename ContainerOfContainer_in>
            constexpr static void Update(const ContainerOfContainer_in & _CC,type & Indexes)noexcept{
                _index_impl::_init_index_count(_CC,Indexes.begin());
            }
        };
    };

    
    namespace cnt_map{

        template <typename Containertype>
        struct mapper{
            template <typename LambdaType>
            static auto map(Containertype &&cnt,LambdaType && F){
                std::vector<typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type> V;
                V.reserve(cnt.size());
                for(size_t i=0;i<V.size();++i){
                    V.push_back(F(cnt[i]));
                }
                return V;
            }
        };

        template <typename T,typename...Args>
        struct mapper<std::vector<T,Args...>>{
            template <typename Containertype,typename LambdaType>
            static auto map(Containertype && cnt,LambdaType && F){
                std::vector<typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type,
                    Args...> V;
                V.reserve(cnt.size());
                for(size_t i=0;i<V.size();++i){
                    V.push_back(F(cnt[i]));
                }
            }
        };

        template <typename T,size_t N>
        struct mapper<std::array<T,N>>{
            template <typename Containertype,typename LambdaType>
            static auto map(Containertype && cnt,LambdaType && F){
                std::array<typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type,N> V;
                for(size_t i=0;i<V.size();++i){
                    V.push_back(F(cnt[i]));
                }
                return V;
            }
        };

        template <typename T>
        struct mapper<ConstValueVector<T>>{
            template <typename Containertype,typename LambdaType>
            static auto map(Containertype && cnt,LambdaType && F){
                return ConstValueVector<
                        typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type
                    >(F(cnt[0]),cnt.size());
            }
        };

        template <typename T>
        struct mapper<ConstReferenceVector<T>>{
            template <typename Containertype,typename LambdaType>
            static auto map(Containertype && cnt,LambdaType && F){
                return ConstValueVector<
                        typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type
                    >(F(cnt[0]),cnt.size());
            }
        };

        template <typename T>
        struct mapper<ConstSharedValueVector<T>>{
            template <typename Containertype,typename LambdaType>
            static auto map(Containertype && cnt,LambdaType && F){
                if(cnt.getPtr() != nullptr){
                    return ConstSharedValueVector<
                            typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type
                        >(cnt.size(),F(cnt[0]) );
                }else{
                    return ConstSharedValueVector<
                            typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type
                        >(cnt.size());
                }
            }
        };
    };

    template <typename Containertype,typename LambdaType>
    auto map(Containertype && C,LambdaType && F){
        return cnt_map::mapper<typename std::decay<Containertype>::type>::map(
            std::forward<Containertype>(C),
            std::forward<LambdaType>(F));
    }
    

};



#endif
