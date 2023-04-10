#ifndef ITERATOR_TEMPLATES_HPP
#define ITERATOR_TEMPLATES_HPP

#include <iterator>

namespace grob{
    template <typename Indexer,typename ret_type>
    struct _const_iterator_template: public std::iterator<
                        std::bidirectional_iterator_tag ,   // iterator_category
                        ret_type,                      // value_type
                        size_t,                      // difference_type
                        ret_type *,               // pointer
                        ret_type                       // reference
    >{
        protected:
        const Indexer * _cnt;
        size_t _index;
        public:
        _const_iterator_template(const Indexer &_cnt,size_t _index):_index(_index),_cnt(&_cnt){}
        
        constexpr inline ret_type operator *() const noexcept(noexcept(_cnt[std::declval<size_t>()])){
            return (*_cnt)[_index];
        }
        constexpr inline ret_type operator [](int i) const noexcept(noexcept(_cnt[std::declval<size_t>()])){
            return (*_cnt)[_index+i];
        }
        inline _const_iterator_template & operator ++() noexcept{++_index;return *this;};
        inline _const_iterator_template operator ++(int)noexcept{
            _const_iterator_template tmp = *this;
            ++_index;
            return tmp;
        };
        inline _const_iterator_template& operator +=(size_t shift)noexcept{
            _index+=shift;
            return *this;
        };
        inline _const_iterator_template& operator -=(size_t shift)noexcept{
            _index-=shift;
            return *this;
        };

        inline _const_iterator_template operator +(int shift)const noexcept{
            return _const_iterator_template(_cnt,_index+shift);
        };
        
        friend inline _const_iterator_template operator +(int shift,const _const_iterator_template cit) noexcept{
            return cit+shift;
        };

        inline _const_iterator_template operator -(int shift) const noexcept{
            return _const_iterator_template(_index-shift,_cnt);
        };

        friend inline bool operator== (const _const_iterator_template& a, const _const_iterator_template& b) noexcept{
             return (a._index == b._index && a._cnt == b._cnt); };
        friend inline bool operator!= (const _const_iterator_template& a, const _const_iterator_template& b) noexcept{
             return (a._index != b._index || a._cnt != b._cnt); };
    };

    template <typename _iterator>
    struct _iterator_base{
        protected:
        _iterator _base;
        public:
        _iterator_base(_iterator _base):_base(_base){}
        inline _iterator_base & operator ++() noexcept{++_base;return *this;};
        inline _iterator_base operator ++(int)noexcept{
            _iterator_base tmp = *this;
            ++_base;
            return tmp;
        };
        inline _iterator_base & operator +=(size_t shift)noexcept{
            _base+=shift;
            return *this;
        };
        inline _iterator_base & operator -=(size_t shift)noexcept{
            _base-=shift;
            return *this;
        };

        inline _iterator_base operator +(int shift) const noexcept{
            return _iterator_base(_base + shift);
        };

        friend inline _iterator_base operator +(int shift, const _iterator_base & _ib) noexcept{
            return _ib+ shift;
        };

        inline _iterator_base operator -(int shift)const noexcept{
            return _iterator_base(_base - shift);
        };

        friend inline bool operator== (const _iterator_base& a, const _iterator_base& b) noexcept{
             return a._base == b._base;}
        friend inline bool operator!= (const _iterator_base& a, const _iterator_base& b) noexcept{
             return a._base != b._base;}
    };


    template <typename Container,typename T,typename Comparator = std::less<T>>
    inline size_t __find_index_sorted_corrector(const Container &X,T const& x,size_t i1,size_t i2, Comparator && cmp = std::less<T>{}){
        while(i1 +1 < i2){
            size_t i = (i1 + i2)/2;
            if(cmp(x,X[i])){
                i2 = i;
            }
            else {
                i1 = i;
            }
        }
        return i1;
    }
    template <typename Container,typename T,typename Comparator = std::less<T>>
    inline size_t __find_index_sorted_with_guess(const Container &X,T const&x, Comparator && cmp = std::less<T>{}){
        size_t N = X.size();
        if(N <= 1 or cmp(x,X[0])){
            return 0;
        }
        size_t i_guess = N*(x-X[0])/(X[N-1]-X[0]);
        if(i_guess >= N-1){
            return N-2;
        }
        if(cmp(x,X[i_guess])){
            size_t di = 1;
            size_t i1 = i_guess-di;
            while(cmp(x,X[i1])){
                if(di < i1){
                    i1 -= di;
                    di *= 2;
                }
                else{
                    i1 = 0;
                    break;
                }
            }
            return __find_index_sorted_corrector(X,x,i1,i_guess,cmp);
        }
        else{
            size_t di = 1;
            size_t i1 = i_guess+di;
            while(cmp(X[i1],x)){
                i1 += di;
                if(i1 >= N){
                    i1 = N;
                    break;
                }
            }
            return __find_index_sorted_corrector(X,x,i_guess,i1,cmp);
        }
    }
    template <typename Container,typename T,typename Comparator>
    inline auto __find_index_sorted(const Container &X,T const&x, Comparator && cmp = std::less<T>{}){
        return __find_index_sorted_corrector(X,x,0,X.size()-1,cmp);
    }
};

#endif