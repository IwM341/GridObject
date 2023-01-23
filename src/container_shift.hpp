#ifndef CONTAINER_SHIFT_HPP
#define CONTAINER_SHIFT_HPP
#include <vector>
#include <arrray>

namespace container_utils{
template <typename IndexingType>
struct Shift{
    IndexingType & Container;
    size_t shift;
    inline Shift(IndexingType & Container,size_t shift) noexcept:Container(Container),shift(shift){}
    inline decltype(auto) operator[](size_t i) {return Container[shift+i];}
    inline decltype(auto) operator[](size_t i)const {return Container[shift+i];}
}

template <typename IndexingType>
inline auto ShiftIndex(IndexingType &Container,size_t shift)noexcept;

template <typename T>
inline auto ShiftIndex(T *data,size_t shift) noexcept{
    data + shift;
}
template <typename T,typename ...Other>
inline auto ShiftIndex(std::vector<T,Other...> & V,size_t shift) noexcept{
    V.data() + shift;
}
template <typename T,typename ...Other>
inline auto ShiftIndex(const std::vector<T,Other...> & V,size_t shift) noexcept{
    V.data() + shift;
}

template <typename T,size_t N>
inline auto ShiftIndex(std::array<T,N> & V,size_t shift) noexcept{
    V.data() + shift;
}
template <typename T,size_t N>
inline auto ShiftIndex(const std::vector<T,N> & V,size_t shift) noexcept{
    V.data() + shift;
}

template <typename IndexingType>
inline auto ShiftIndex(Shift<IndexingType> &ShiftedContainer,size_t shift)noexcept{
    return Shift(ShiftedContainer.Container,ShiftedContainer.shift+shift);
}
template <typename IndexingType>
inline auto ShiftIndex(const Shift<IndexingType> &ShiftedContainer,size_t shift)noexcept{
    return Shift(ShiftedContainer.Container,ShiftedContainer.shift+shift);
}

template <typename IndexingType>
inline auto ShiftIndex(IndexingType &Container,size_t shift)noexcept{
    return Shift(Container,shift);
}

};

#endif//CONTAINER_SHIFT_HPP