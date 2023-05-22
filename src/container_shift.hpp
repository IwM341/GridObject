#ifndef CONTAINER_SHIFT_HPP
#define CONTAINER_SHIFT_HPP
#include <vector>
#include <array>

namespace grob{

template <typename T>
struct vector_view{
    typedef typename std::decay<T>::type value_type;
    T * values;
    size_t _size;
    inline constexpr vector_view(T * values = nullptr,size_t _size = 0)noexcept:values(values),_size(_size){}
    inline constexpr size_t size() const noexcept{return _size;}

    
    inline operator T * ()noexcept{return values;}
    inline operator const T * () const noexcept{return values;}
    
    typedef T * iterator;
    typedef const T * const_iterator;
    inline T *  begin() noexcept{return values;}
    inline T *  end() noexcept{return values+_size;}
    inline const T *  cbegin()const noexcept{return values;}
    inline const T *  cend()const noexcept{return values+_size;}
    inline const T *  begin()const noexcept{return cbegin();}
    inline const T *  end()const noexcept{return cend();}
};

template <typename Container>
struct container_iterator{
    private:
    Container & cnt_;
    size_t index_;
    public:
    inline container_iterator(Container & cnt_,size_t index_)noexcept :cnt_(cnt_),index_(index_){};
    inline decltype(auto) operator*() noexcept(noexcept(cnt_[std::declval<size_t>()]))
        {return cnt_[index_];}
    inline decltype(auto) operator*()const noexcept(noexcept((Container const &)(cnt_)[std::declval<size_t>()]))
        {return (Container const &)(cnt_)[index_];}

    inline decltype(auto) operator[](int i)noexcept(noexcept(cnt_[std::declval<size_t>()]))
        {return cnt_[index_+i];}
    inline decltype(auto) operator[](int i)const noexcept(noexcept((const Container &)(cnt_)[std::declval<size_t>()])) 
        {return (const Container &)(cnt_)[index_+i];}

    container_iterator & operator ++(){++index_;return *this;}
    container_iterator  operator ++(int){
        container_iterator tmp(cnt_,index_);
        ++index_;
        return tmp;
    }
    
    inline container_iterator &  operator +=(int i) noexcept{
        index_ += i;
        return *this;
    }
    inline container_iterator &  operator -=(int i) noexcept{
        index_ -= i;
        return *this;
    }
    inline container_iterator  operator +(int i) noexcept{
        return container_iterator (cnt_,index_ +i);
    }
    inline container_iterator  operator -(int i) noexcept{
        return container_iterator (cnt_,index_ - i);
    }
    friend inline bool operator ==(const container_iterator & s1,const container_iterator & s2) noexcept{
        return (s1.index_ == s2.index_ && &s1.cnt_ == &s2.cnt_);
    }
    friend inline bool operator !=(const container_iterator & s1,const container_iterator & s2) noexcept{
        return (s1.index_ != s2.index_ || &s1.cnt_ != &s2.cnt_);
    } 
};

template <typename ContainerType>
struct container_slice{
    ContainerType * _container;
    size_t _begin;
    size_t _size;
    typedef typename std::decay<decltype((*_container)[std::declval<size_t>()])>::type value_type;
    typedef decltype((*_container)[std::declval<size_t>()]) ref_type;

    inline container_slice(ContainerType *_container,size_t _begin = 0,size_t _size = 0)noexcept:
        _container(&_container),_begin(_begin),_size(_size){}
    inline size_t size() const noexcept{return _size;}
    
    inline  const ref_type operator[](size_t i)const noexcept(noexcept((*_container)[std::declval<size_t>()])) 
        {return (*_container)[_begin+i];}
    inline ref_type operator[](size_t i)noexcept(noexcept((*_container)[std::declval<size_t>()]))
        {return (*_container)[_begin+i];}
    
    friend inline bool operator ==(const container_slice & s1,const container_slice & s2) noexcept{
        return (s1._begin == s2._begin && s1._container == s2._container && s1._size == s2._size);
    }
    friend inline bool operator !=(const container_slice & s1,const container_slice & s2) noexcept{
        return (s1._begin != s2._begin || s1._container != s2._container || s1._size != s2._size);
    }

    typedef container_iterator<ContainerType> iterator;
    typedef container_iterator<const ContainerType> const_iterator;

    inline iterator begin() noexcept{return iterator(_container,_begin);}
    inline iterator end() noexcept{return iterator(_container,_begin+_size);}
    inline const_iterator cbegin()const noexcept{return const_iterator(_container,_begin);}
    inline const_iterator cend()const noexcept{return const_iterator(_container,_begin+_size);}
    inline const_iterator begin()const noexcept{return const_iterator(_container,_begin);}
    inline const_iterator end()const noexcept{return const_iterator(_container,_begin+_size);}
};

template <typename Container>
struct container_slice<container_slice<Container>>:container_slice<Container>{
    using container_slice<Container>::container_slice;
    container_slice<container_slice<Container>>(container_slice<Container> cnt,size_t _shift,size_t _size):
        container_slice<Container>(cnt._container,cnt._begin+_shift,_size){}
};

template <typename IndexingType>
struct Shift{
    typedef IndexingType ContainerType;
    typedef IndexingType & ContainerType_ref;

    IndexingType & container;
    size_t shift;
    inline Shift(IndexingType & container,size_t shift) noexcept:container(container),shift(shift){}
    inline decltype(auto) operator[](size_t i)noexcept(noexcept(container[std::declval<size_t>()]))
        {return container[shift+i];}
    inline decltype(auto) operator[](size_t i)const noexcept(noexcept(container[std::declval<size_t>()])) 
        {return (const IndexingType &)(container)[shift+i];}
    
    friend inline bool operator ==(const Shift & s1,const Shift & s2) noexcept{
        return (s1.shift == s2.shift && &s1.container == &s2.container);
    }
    friend inline bool operator !=(const Shift & s1,const Shift & s2) noexcept{
        return (s1.shift != s2.shift || &s1.container != &s2.container);
    } 

};


template <typename IndexingType>
inline auto make_slice(IndexingType &Container,size_t shift,size_t size = 0)noexcept{
    return container_slice<IndexingType>(&Container,shift,size);
}


template <typename T>
inline auto make_slice(vector_view<T> &Container,size_t shift,size_t size = 0)noexcept{
    return vector_view<T>(Container.values + shift,size);
}

template <typename T>
inline auto make_slice(const vector_view<T> &Container,size_t shift,size_t size = 0)noexcept{
    return vector_view<const T>(Container.values + shift,size);
}

template <typename T,typename ...Other>
inline auto make_slice(std::vector<T,Other...> & V,size_t shift,size_t size = 0) noexcept{
    return vector_view<T>(V.data() + shift,size);
}
template <typename T,typename ...Other>
inline auto make_slice(const std::vector<T,Other...> & V,size_t shift,size_t size = 0) noexcept{
    return vector_view<const T>(V.data() + shift,size);
}

template <typename T,size_t N>
inline auto make_slice(std::array<T,N> & V,size_t shift,size_t size = 0) noexcept{
    return vector_view<const T>(V.data() + shift,size);
}
template <typename T,size_t N>
inline auto make_slice(const std::array<T,N> & V,size_t shift,size_t size = 0) noexcept{
    return vector_view<const T>(V.data() + shift,size);
}
};

#endif//CONTAINER_SHIFT_HPP