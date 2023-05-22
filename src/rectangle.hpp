#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include <vector>
#include <array>
#include <sstream>
#include "object_serialization.hpp"

/*!
*/
namespace grob{

    template <typename Rect1,typename Rect2,typename A,typename B>
    inline constexpr auto linear_combination(Rect1 const & R1,Rect2 const & R2,A const &a,B const & b) noexcept;

    template <typename T>
    struct Rect{
        constexpr static size_t Dim = 1;
        T left_, right_;
        public:
        
        inline constexpr Rect()noexcept{}
        inline constexpr Rect(T left_,T right_) noexcept:left_(left_),right_(right_){}

        inline constexpr const T & left() const noexcept{return left_;}
        inline constexpr const T & right() const noexcept{return right_;}

        inline constexpr T & left() noexcept{return left_;}
        inline constexpr T & right() noexcept{return right_;}

        template <uint8_t level,uint8_t b>
        constexpr const inline T & get()const noexcept{
            static_assert(!level,"too big level of recursion in Rectangle");
            return (b ? right_ : left_);
        }
        
        template <uint8_t level,uint8_t b = 0>
        constexpr const inline auto & getR()const noexcept{
            static_assert(!level,"too big level of recursion in Rectangle");
            return *this;
        }
    
        template <uint8_t level,uint8_t b>
        constexpr inline T & get() noexcept
        {
            return const_cast<T &>(static_cast<const Rect &>(*this).get<level,b>());
        }

        template <uint8_t level,uint8_t b = 0>
        constexpr inline auto & getR() noexcept
        {
            return const_cast<Rect &>(static_cast<const Rect &>(*this).getR<level,b>());
        }
        
        inline constexpr bool contains(T const & x) const noexcept{
            return left_ <= x && x<= right_;
        }
        template <typename U>
        inline auto interpol_coeff (U const &x) const noexcept{
            return (right_-x)/(right_-left_);
        }
    
        template <typename U>
        inline auto reduction (U const &a,U const &b) const noexcept{
            return a*left_+b*right_;
        }

        template <size_t tuple_index = 0,typename...Args>
        inline constexpr bool contains_tuple(std::tuple<Args...> const & X) const noexcept{
            return left_ <= std::get<tuple_index>(X) && std::get<tuple_index>(X) <= right_;
        }

        inline constexpr T volume()const noexcept{return right_-left_;}
        T inline constexpr center()const noexcept{return (right_+left_)/2;}

        friend std::ostream & operator << (std::ostream & os,const Rect & R){
            std::ostringstream internal_stream;
            internal_stream << "Rect(" <<R.left() << ", " <<R.right()<< ")";
            return os << internal_stream.str();
        }


        inline constexpr Rect & operator *= (const T &multiplier)noexcept{
            left_ *=multiplier;
            right_ *=multiplier;
            return *this;
        }
        
        inline constexpr Rect & operator /= (const T &divisor)noexcept{
            return this->operator*=(1/divisor);
        }

        template <typename RectType>
        inline constexpr Rect & operator += (const RectType &second)noexcept{
            left_+= second.left();
            right_ += second.right();
            return *this;
        }

        template <typename RectType>
        inline constexpr Rect & operator -= (const RectType &second)noexcept{
            left_ -= second.left();
            right_ -= second.right();
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
        Ret.left_ = a*R1.left_+b*R2.left_;
        Ret.right_ = a*R1.right_+b*R2.right_;
        return Ret;
    }


    template <typename T,typename InnerRect>
    struct RectCommon{
        constexpr static size_t Dim = 1 + InnerRect::Dim;

        inline constexpr RectCommon()noexcept{}
        inline constexpr RectCommon(T left_,T right_,
            InnerRect inner_left_,
            InnerRect inner_right_) noexcept:
            first_(left_, right_),inner_left_(inner_left_),inner_right_(inner_right_){}
        inline constexpr RectCommon(Rect<T> first_,
            InnerRect inner_left_,
            InnerRect inner_right_) noexcept:
            first_(first_),inner_left_(inner_left_),inner_right_(inner_right_){}

        template <size_t level,size_t b,
                  typename std::enable_if<level!=0,bool>::type = true>
        constexpr inline auto const& get() const noexcept{
            (b&1 ? right().template get<level-1,(b>>1)>(): left().template get<level-1,(b>>1)>());
        }

        template <size_t level,size_t b,
                  typename std::enable_if<level==0,bool>::type = true>
        constexpr inline auto const& get() const noexcept{
            return first_.template get<level,b>();
        }

        template <size_t level,size_t b> 
        constexpr inline auto & get() noexcept{
                return const_cast<T &>(static_cast<const RectCommon &>(*this).get<level,b>());
        }

        template <size_t level,size_t b = 0,
                  typename std::enable_if<level!=0,bool>::type = true>
        constexpr inline auto const& getR() const noexcept{
            (b&1 ? right().template getR<level-1,(b>>1)>(): left().template getR<level-1,(b>>1)>());
        }
        template <size_t level,size_t b = 0,
                  typename std::enable_if<level==0,bool>::type = true>
        constexpr inline auto const& getR() const noexcept{
            return first_;
        }

        template <uint8_t level,uint8_t b>
        constexpr inline auto & getR() noexcept
        {
            return const_cast<Rect<T> &>(static_cast<const RectCommon &>(*this).getR<level,b>());
        }

        inline constexpr auto const& first() const noexcept{return first_;}
        inline constexpr auto & first() noexcept{return first_;}

        inline constexpr const T & left() const noexcept{return first_.left_;}
        inline constexpr const T & right() const noexcept{return first_.right_;}

        inline constexpr T & left() noexcept{return first_.left_;}
        inline constexpr T & right() noexcept{return first_.right_;}

        inline constexpr const InnerRect & inner_left() const noexcept{return inner_left_;}
        inline constexpr const InnerRect & inner_right() const noexcept{return inner_right_;}

        inline constexpr InnerRect & inner_left() noexcept{return inner_left_;}
        inline constexpr InnerRect & inner_right() noexcept{return inner_right_;}

        inline constexpr InnerRect volume()const noexcept{return first_.volume()*(inner_left_.volume()+inner_right_.volume())/2;}

        auto inline constexpr center()const noexcept{
            return std::make_tuple(first_.center());
        }

        template <typename...Args>
        inline constexpr bool contains(Args const &... args) const noexcept{
            return contains_tuple<0>(std::make_tuple(args...));
        }

        template <size_t tuple_index = 0,typename...Args>
        inline constexpr bool contains_tuple(std::tuple<Args...> const & X) const noexcept{
            auto ic = first_.interpol_coeff(std::get<tuple_index>(X));
            if(ic < 0 || ic > 1){
                return false;
            }
            auto _average = linear_combination(inner_left_,inner_right_,ic,1-ic);
            return _average.template contains_tuple<tuple_index+1>(X);
        }

        template <typename U>
        inline constexpr auto inner_reduce(U const & a)const noexcept{
            //auto ic = first_.interpol_coeff(a);
            return linear_combination(inner_left_,inner_right_,a,1-a);
        }


        inline constexpr RectCommon & operator *= (const T &multiplier)noexcept{
            first_ *= multiplier;
            inner_left_ *= multiplier;
            inner_right_ *= multiplier;
            return *this;
        }
        
        inline constexpr RectCommon & operator /= (const T &divisor)noexcept{
            return this->operator*=(1/divisor);
        }

        template <typename RectType>
        inline constexpr RectCommon & operator += (const RectType &second)noexcept{
            first_.left_ += second.left();
            first_.right_ += second.right();
            inner_left_ += second.inner_left();
            inner_right_ += second.inner_right();
            return *this;
        }

        template <typename RectType>
        inline constexpr RectCommon & operator -= (const RectType &second)noexcept{
            first_.left_ -= second.left();
            first_.right_ -= second.right();
            inner_left_ -= second.inner_left();
            inner_right_ -= second.inner_right();
            return *this;
        }
        
        friend std::ostream & operator << (std::ostream & os,const RectCommon & R){
            std::ostringstream internal_stream;
            internal_stream << "RectCommon( Rect(" <<R.left() << ", " <<R.right() << ")," <<R.inner_left_ << "," << R.inner_right_ << ")";
            return os << internal_stream.str();
        }


        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(3,
                [](size_t i){
                    switch (i){
                    case 0:
                        return "first";
                        break;
                    case 1:
                        return "inner_left";
                        break;
                    default:
                        return "inner_right";
                        break;
                    }
                },
                [this,&S](size_t i){
                    switch (i)
                    {
                    case 0:
                        return first_.Serialize(S);
                        break;
                    case 1:
                        return inner_left_.Serialize(S);
                        break;
                    default:
                        return inner_right_.Serialize(S);
                        break;
                    }
            });
        }
        template <typename ObjecType,typename DeSerializer>
        void  init_serialize(ObjecType && Object,DeSerializer && S){
            first_.init_serialize(S.GetProperty(Object,"first"));
            inner_left_.init_serialize(S.GetProperty(Object,"inner_left"));
            inner_right_.init_serialize(S.GetProperty(Object,"inner_right"));
        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w)const{
            first().write(w);
            inner_left().write(w);
            inner_right().write(w);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            first().init_read(r);
            inner_left().init_read(r);
            inner_right().init_read(r);
        }
        OBJECT_DESERIALIZATION_FUNCTION(RectCommon)
        OBJECT_READ_FUNCTION(RectCommon)

        
        Rect<T> first_;
        InnerRect inner_left_,inner_right_;
        

    };

    template <typename U,typename InnerU,typename Rect2,typename A,typename B>
    inline constexpr auto linear_combination(RectCommon<U,InnerU> const & R1,Rect2 const & R2,A const & a,B const & b) noexcept{
        typedef  decltype(std::declval<U>()*a+R2.left()*b) UV;
        typedef decltype(linear_combination(std::declval<InnerU>(),R2.inner_left(),a,b)) InnerUV;
        RectCommon<UV,InnerUV> Ret;
        Ret.first = linear_combination(R1.first_,R2.first_a,b);
        Ret.inner_left_ = linear_combination(R1.inner_left_,R2.inner_left(),a,b);
        Ret.inner_right_ = linear_combination(R1.inner_right_,R2.inner_right(),a,b);
        return Ret;
    }

    template <typename T,typename InnerRect>
    struct RectConst{
        constexpr static size_t Dim = 1 + InnerRect::Dim;


        inline constexpr RectConst()noexcept{}
        inline constexpr RectConst(T left,T right,
            InnerRect inner_left_right) noexcept:
            first_(left, right),inner_left_right_(inner_left_right){}
        inline constexpr RectConst(Rect<T> first_,
            InnerRect inner_left_right) noexcept:
            first_(first_),inner_left_right_(inner_left_right){}
        

        template <uint8_t level,uint8_t b>
        constexpr inline T const& get() const noexcept{
            static_assert(level < Dim,"error: level >= Dim");
                return (level ? inner_left_right_.template get<level-1,(b>>1)>():
                    (b ? right() : left()));
            }
        
        template <uint8_t level,uint8_t b>
        constexpr inline T & get() noexcept{
                return const_cast<T &>(static_cast<const RectConst &>(*this).get<level,b>());
            }
        template <size_t level,size_t b = 0,
                  typename std::enable_if<level!=0,bool>::type = true>
        constexpr inline auto const& getR() const noexcept{
            return inner_left_right_.template getR<level-1,(b>>1)>();
        }
        template <size_t level,size_t b = 0,
                  typename std::enable_if<level==0,bool>::type = true>
        constexpr inline auto const& getR() const noexcept{
            return first_;
        }

        template <uint8_t level,uint8_t b>
        constexpr inline auto & getR() noexcept
        {
            return const_cast<Rect<T> &>(static_cast<const RectConst &>(*this).getR<level,b>());
        }

        
        inline constexpr auto const& first() const noexcept{return first_;}
        inline constexpr auto & first() noexcept{return first_;}

        inline constexpr InnerRect const& inner() const noexcept{return inner_left_right_;}
        inline constexpr InnerRect & inner() noexcept{return inner_left_right_;}

        inline constexpr const T & left() const noexcept{return first_.left();}
        inline constexpr const T & right() const noexcept{return first_.right();}

        inline constexpr T & left() noexcept{return first_.left();}
        inline constexpr T & right() noexcept{return first_.right();}

        inline constexpr const InnerRect & inner_left() const noexcept{return inner_left_right_;}
        inline constexpr const InnerRect & inner_right() const noexcept{return inner_left_right_;}

        inline constexpr InnerRect & inner_left() noexcept{return inner_left_right_;}
        inline constexpr InnerRect & inner_right() noexcept{return inner_left_right_;}

        inline constexpr T volume()const noexcept{return (right()-left())*inner_left_right_.volume();}


         template <typename U>
        inline constexpr decltype(auto) inner_reduce(U const & a)const noexcept{
            return inner_left_right_;
        }
        template <size_t tuple_index = 0,typename...Args>
        inline constexpr bool contains_tuple(std::tuple<Args...> const & X) const noexcept{
            return first_.contains(std::get<tuple_index>(X)) && inner_left_right_.template contains_tuple<tuple_index+1>(X);
        }
        


        inline constexpr RectConst & operator *= (const T &multiplier)noexcept{
            first_.left_ *= multiplier;
            inner_left_right_ *= multiplier;
            return *this;
        }
        
        inline constexpr RectConst & operator /= (const T &divisor)noexcept{
            return this->operator*=(1/divisor);
        }

        template <typename RectType>
        inline constexpr RectConst & operator += (const RectType &second)noexcept{
            left() += second.left();
            right() += second.right();
            inner_left_right_ += second.inner_left();
            return *this;
        }

        template <typename RectType>
        inline constexpr RectConst & operator -= (const RectType &second)noexcept{
            left() -= second.left();
            right() -= second.right();
            inner_left_right_ -= second.inner_left();
            return *this;
        }


        friend std::ostream & operator << (std::ostream & os,const RectConst & R){
            std::ostringstream internal_stream;
            internal_stream << "RectConst( Rect(" <<R.left() << ", " <<R.right() << ")," <<R.inner_left_right_ << ")";
            return os << internal_stream.str();
        }


        template <typename Serializer>
        auto Serialize(Serializer && S)const{
            return S.MakeDict(2,
                [](size_t i){ return (i?"inner":"first");},
                [this,&S](size_t i){
                    return ( !i? first_.Serialize(S) : 
                        inner_left_right_.Serialize(S));
                    }
            );
        }
        template <typename ObjecType,typename DeSerializer>
        void  init_serialize(ObjecType && Object,DeSerializer && S){
            first().init_serialize(S.GetProperty(Object,"first"));
            inner_left_right_.init_serialize(S.GetProperty(Object,"inner"));
        }
        template <typename WriterStreamType>
        void write(WriterStreamType && w)const{
            first().write(w);
            inner().write(w);
        }
        template <typename ReaderStreamType>
        void init_read(ReaderStreamType && r){
            first().init_read(r);
            inner().init_read(r);
        }
        OBJECT_DESERIALIZATION_FUNCTION(RectConst)
        OBJECT_READ_FUNCTION(RectConst)

        
        Rect<T> first_;
        InnerRect inner_left_right_;
    };

    template <typename U,typename InnerU,typename V,typename InnerV,typename A,typename B>
    inline constexpr auto linear_combination(RectConst<U,InnerU> const & R1,RectConst<V,InnerV> const & R2,A const & a,B const & b) noexcept{
        typedef  decltype(std::declval<U>()*a+std::declval<V>()*b) UV;
        typedef decltype(linear_combination(std::declval<InnerU>(),std::declval<InnerV>(),a,b)) InnerUV ;
        RectConst<UV,InnerUV> Ret;
        Ret.first = linear_combination(R1.first_,R2.first_a,b);
        Ret.inner_left_right_ = linear_combination(R1.inner_left_right_,R2.inner_left_right_,a,b);
        return Ret;
    }
    template <typename U,typename InnerU,typename V,typename InnerV,typename A,typename B>
    inline constexpr auto linear_combination(RectConst<U,InnerU> const & R1,RectCommon<U,InnerU> const & R2,A const & a,B const & b) noexcept{
        return linear_combination(R2,R1,b,a);
    }


    
    template <typename T,typename InnerRect>
    auto make_rect(Rect<T> const & first,InnerRect const & left,InnerRect const & right){
        return RectCommon<T,InnerRect>(first,left,right);
    }
    
    template <typename T,typename InnerRect>
    auto make_rect(Rect<T> const & first,InnerRect const & inner){
        return RectConst<T,InnerRect>(first,inner);
    }
};

#endif//RERCANGLE_HPP
