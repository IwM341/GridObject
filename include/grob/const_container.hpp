#ifndef CONST_CONTAINER_HPP
#define CONST_CONTAINER_HPP

#include <memory>
#include <iostream>
#include <sstream>
#include "object_serialization.hpp"
#include "iterator_template.hpp"
namespace grob{
    
    template <typename T>
    class ConstValueVector{
        
        T value;
        size_t _size;

        public:
        typedef T value_type;

        inline constexpr size_t size() const noexcept{
            return _size;
        }
        inline constexpr T const& operator[](size_t) const noexcept{
            return value;
        }
        inline constexpr T & operator[](size_t)  noexcept{
            return value;
        }

        ConstValueVector(size_t _size = 0):_size(_size){}
        inline constexpr ConstValueVector(T value,size_t _size = 0) noexcept:
            value(std::forward<T>(value)),
            _size(_size) {}
        
        inline void resize(size_t new_size)noexcept{_size = new_size;}

        inline constexpr T & first() noexcept{return value;}
        inline constexpr T const& first() const noexcept{return value;}

        inline constexpr T & back() noexcept{return value;}
        inline constexpr T const& back() const noexcept{return value;}


        friend std::ostream & operator << (std::ostream & os,ConstValueVector const&V){
            std::ostringstream S;
            S << "ConstValueVector[" << V.value << "]";
            return os << S.str();
        }


        SERIALIZATOR_FUNCTION(
            PROPERTY_NAMES("size","value"),
            PROPERTIES(_size,value)
        )
        WRITE_FUNCTION(_size,value)
        DESERIALIZATOR_FUNCTION(
            ConstValueVector,
            PROPERTY_NAMES("size"),
            PROPERTY_TYPES(_size)
        )

        READ_FUNCTION(ConstValueVector,
            PROPERTY_TYPES(_size,value)
        )
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


        SERIALIZATOR_FUNCTION(
            PROPERTY_NAMES("size","value"),
            PROPERTIES(size_,*value_ptr)
        )
        WRITE_FUNCTION(size_,*value_ptr)
            

    };

    template <typename FuncType>
    struct MappedContainer{
        typedef decltype(std::declval<FuncType>()(std::declval<size_t>())) value_type;
        FuncType F;
        size_t _size;
        MappedContainer(FuncType F,size_t _size):F(std::forward<FuncType>(F)),_size(_size){}
        inline size_t size()const{
            return _size;
        }
        
        decltype(auto) operator [](size_t i){return F(i);}
        decltype(auto) operator [](size_t i) const{return F(i);}
        
        /// @brief iterator class
        typedef _const_iterator_template<MappedContainer,value_type> const_iterator;

        /// @brief 
        /// @return 
        inline const_iterator begin()const noexcept{return const_iterator(*this,0);}
        /// @brief 
        /// @return 
        inline const_iterator end()const noexcept{return const_iterator(*this,_size);}
        /// @brief 
        /// @return 
        inline const_iterator cbegin()const noexcept{return begin();}
        /// @brief 
        /// @return 
        inline const_iterator cend()const noexcept{return end();}

    };
    
    /// @brief makes vector view of function, so that V[i] = F(i)
    /// @param size size of container
    /// @param F ravlue ref, [](size_t)->T
    /// @return abstract object with [] operator
    /// the value of F forwarded into container.
    template <typename FuncType>
    auto as_container(FuncType &&F,size_t size){
        return MappedContainer<
            typename std::decay<FuncType>::type
        >(std::forward<FuncType>(F),size);
    }

    /// @brief makes vector view of function, so that V[i] = F(i)
    /// @param size size of container
    /// @param F is ref of [](size_t)->T
    /// @return abstract object with [] operator
    /// the reference to F is stored: ATTENTION possible bad ref
    template <typename FuncType>
    auto as_container(FuncType &F,size_t size){
        return MappedContainer<FuncType &>(F,size);
    }

    template <typename Container>
    auto as_func(Container && C){
        return [_C = std::forward<Container>(C)](size_t i){
            return i;
        };
    }
    template <typename FuncType>
    auto as_func(MappedContainer<FuncType> && C){
        return std::move(C.F);
    } 
    template <typename FuncType>
    auto & as_func(MappedContainer<FuncType> & C){
        return C.F;
    }
    template <typename FuncType>
    const auto & as_func(MappedContainer<FuncType> const& C){
        return C.F;
    } 


    struct LinearIndexer{
        size_t size_;
        size_t len_;
        
        typedef size_t value_type;
        LinearIndexer(size_t size_ = 0,size_t len_ = 1):size_(size_),len_(len_){}
        size_t inline constexpr  operator [](size_t i)const noexcept{return i*len_;}
        size_t inline constexpr  size()const noexcept{return size_;}

        inline constexpr size_t first()const noexcept{return 0;}
        inline constexpr size_t back()const noexcept{return len_*(size_-1);}

        constexpr inline size_t find(size_t i) const noexcept{
            return i/len_;
        }
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

    /// @brief makes vector from function, so that V[i] = F(i)
    /// @param size size of container
    /// @param F [](size_t)->T
    /// @return std::vector<T>
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

        template <typename IntContainer>
        inline size_t find_index(IntContainer const& cnt,size_t i){
            return __find_int_index_sorted_with_guess(cnt,i);
        }

        inline size_t find_index(LinearIndexer const& cnt,size_t i){
            return cnt.find(i);
        }

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
                Indexes.resize(_CC.size()+1);
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

            static size_t Find(LinearIndexer const & _cnt,size_t i){
                return find_index(_cnt,i);
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
            template <typename Container_t,typename LambdaType>
            static auto map(Container_t &&cnt,LambdaType && F){
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
                std::vector<typename std::decay<decltype(F(cnt[std::declval<size_t>()]))>::type> V;
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
