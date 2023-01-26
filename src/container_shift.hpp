#ifndef CONTAINER_SHIFT_HPP
#define CONTAINER_SHIFT_HPP
#include <vector>
#include <array>

namespace indexshift{
template <typename IndexingType>
struct Shift{
    typedef IndexingType ContainerType;
    typedef IndexingType & ContainerType_ref;

    IndexingType & container;
    size_t shift;
    inline Shift(IndexingType & container,size_t shift) noexcept:container(container),shift(shift){}
    inline decltype(auto) operator[](size_t i){return container[shift+i];}
    inline decltype(auto) operator[](size_t i)const {return container[shift+i];}
};

template <typename IndexingType>
inline auto ShiftIndex(IndexingType &Container,size_t shift)noexcept;

template <typename T>
inline auto ShiftIndex(T *data,size_t shift) noexcept{
    return data + shift;
}
template <typename T,typename ...Other>
inline auto ShiftIndex(std::vector<T,Other...> & V,size_t shift) noexcept{
    return V.data() + shift;
}
template <typename T,typename ...Other>
inline auto ShiftIndex(const std::vector<T,Other...> & V,size_t shift) noexcept{
    return V.data() + shift;
}

template <typename T,size_t N>
inline auto ShiftIndex(std::array<T,N> & V,size_t shift) noexcept{
    return V.data() + shift;
}
template <typename T,size_t N>
inline auto ShiftIndex(const std::array<T,N> & V,size_t shift) noexcept{
    return V.data() + shift;
}

template <typename IndexingType>
inline auto ShiftIndex(Shift<IndexingType> &ShiftedContainer,size_t shift)noexcept{
    return Shift<IndexingType>(ShiftedContainer.container,ShiftedContainer.shift+shift);
}
template <typename IndexingType>
inline auto ShiftIndex(const Shift<IndexingType> &ShiftedContainer,size_t shift)noexcept{
    return  Shift<const IndexingType>(ShiftedContainer.container,ShiftedContainer.shift+shift);
}

template <typename IndexingType>
inline auto ShiftIndex(IndexingType &Container,size_t shift)noexcept{
    return Shift<IndexingType>(Container,shift);
}

};

#endif//CONTAINER_SHIFT_HPP