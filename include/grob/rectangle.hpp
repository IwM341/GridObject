#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include <sstream>
#include "object_serialization.hpp"

/*!
*/
namespace grob{

    template <typename Rect1,typename Rect2,typename A,typename B>
    inline constexpr auto linear_combination(Rect1 const & R1,Rect2 const & R2,A const &a,B const & b) noexcept;

    template <typename T>
    struct Rect{
        T left, right;
        public:
        
        inline constexpr Rect()noexcept{}
        inline constexpr Rect(T left,T right) noexcept:left(left),right(right){}
        
        inline constexpr bool contains(T const & x) const noexcept{
            return left <= x && x<= right;
        }
        template <typename U>
        inline auto interpol_coeff (U const &x) const noexcept{
            return (right - x)/(right-left);
        }
        
        /// @brief liner combination of left, right
        /// @return a*left+b*right
        template <typename U>
        inline auto reduction (U const &a,U const &b) const noexcept{
            return a*left+b*right;
        }
        
        /// @brief liner combination of left, right
        /// @return (1-a)*left+a*right
        template <typename U>
        inline auto reduction(U const& a) const noexcept {
            return (1-a) * left + a * right;
        }

        inline constexpr T volume()const noexcept{return right-left;}
        T inline constexpr center()const noexcept{return (right+left)/2;}

        friend std::ostream & operator << (std::ostream & os,const Rect & R){
            std::ostringstream internal_stream;
            internal_stream << "Rect(" <<R.left<< ", " <<R.right<< ")";
            return os << internal_stream.str();
        }


        inline constexpr Rect & operator *= (const T &multiplier)noexcept{
            left *=multiplier;
            right *=multiplier;
            return *this;
        }
        
        inline constexpr Rect & operator /= (const T &divisor)noexcept{
            return this->operator*=(1/divisor);
        }

        template <typename RectType>
        inline constexpr Rect & operator += (const RectType &second)noexcept{
            left+= second.left;
            right+= second.right;
            return *this;
        }

        template <typename RectType>
        inline constexpr Rect & operator -= (const RectType &second)noexcept{
            left -= second.left;
            right -= second.right;
            return *this;
        }

        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(2,
                [](size_t i){
                return ( !i ? "left" : "right" );
            },
                [this,&S](size_t i){
                return ( !i ? S.MakePrimitive(left()) : S.MakePrimitive(right()) );
            });
        }
        template <typename ObjecType,typename DeSerializer>
        void init_serialize(ObjecType && Object,DeSerializer && S){
            S.GetPrimitive(S.GetProperty(Object,"left"),left());
            S.GetPrimitive(S.GetProperty(Object,"right"),right());
        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w){
            w.write(left());
            w.write(right());
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            r.read(left());
            r.read(right());
        }
        OBJECT_DESERIALIZATION_FUNCTION(Rect)
        OBJECT_READ_FUNCTION(Rect)
    };

    template <typename U,typename V,typename A,typename B>
    inline constexpr auto linear_combination(Rect<U> const & R1,Rect<V> const & R2,A const & a,B const & b) noexcept{
        typedef  decltype(std::declval<U>()*a+std::declval<V>()*b) UV;
        Rect<UV> Ret;
        Ret.left = a*R1.left+b*R2.left;
        Ret.right = a*R1.right+b*R2.right;
        return Ret;
    }
    template <typename U1, typename U2>
    inline constexpr auto intersect(Rect<U1> const & R1,Rect<U2> const & R2){
        using U = decltype(std::declval<U1>() + std::declval<U2>());
        Rect<U> ret(std::max((U)R1.left,(U)R2.left),
                                    std::min((U)R1.right,(U)R2.right));
        bool b = ret.left <= ret.right;;
        return std::make_pair(ret,b);
    }

};



#endif//RERCANGLE_HPP
