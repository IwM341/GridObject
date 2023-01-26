#include <iostream>
#include "../src/container_shift.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>

template <typename T> 
class OneValueContainer{
    T value_;
    size_t size_;
    public:
    constexpr size_t  size()const noexcept{return size_;}
    constexpr OneValueContainer(T value,size_t size) noexcept:value_(value),size_(size){}
    constexpr inline T & operator[](size_t i)noexcept{return value_;}
    constexpr inline T operator[](size_t i)const noexcept{std::cout << "const operator[]" <<std::endl;return value_;}
};

template <typename T> 
struct ExceptOneValueContainer{
    T value_;
    size_t size_;
    public:
    constexpr size_t  size()const noexcept{return size_;}
    constexpr ExceptOneValueContainer(T value,size_t size):value_(value),size_(size_){}
    constexpr inline T & operator[](size_t i){return value_;}
    constexpr inline T operator[](size_t i)const {std::cout << "const operator[]" <<std::endl;return value_;}
};



int main(){
    const OneValueContainer<double> C0(0.0,10);
    std::vector<double> C1(10,3.14);
    double * C2 = new double[10];
    for(size_t i=0;i<10;++i){
        C1[i] = i;
        C2[i] = i;
    }

    auto V0 = indexshift::ShiftIndex(C0,4);
    auto V1 = indexshift::ShiftIndex(C1,4);
    auto V2 = indexshift::ShiftIndex(C1,4);

    auto VV0 = indexshift::ShiftIndex(V0,4);
    auto VV1 = indexshift::ShiftIndex(V1,4);
    auto VV2 = indexshift::ShiftIndex(V1,4);

    std::cout << TypeToString(decltype(VV0)) <<std::endl;
    std::cout << TypeToString(decltype(VV1)) <<std::endl;
    std::cout << TypeToString(decltype(VV2)) <<std::endl;

    std::cout << V0[4] <<std::endl;
    std::cout << V1[4]<<std::endl;
    std::cout << V2[4] <<std::endl;

    return 0;
}