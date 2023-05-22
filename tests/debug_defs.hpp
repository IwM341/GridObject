#ifndef DEBUG_DEFS_HPP
#define DEBUG_DEFS_HPP

#include <sstream>
#include <tuple>
#include <vector>


namespace debugdefs{


    template <typename T>
    std::string to_debug_string(const T &x){
        std::ostringstream os;
        os <<  x;
        return os.str();
    }
    template <typename...Args>
    std::string to_debug_string(std::vector<Args...> const&V){
        std::ostringstream os;
        os <<  "vector[";
        for(size_t i=0;i<V.size();++i){
            if(i)
                os << ", " <<  V[i];
            else
                os <<  V[i];
        }
        os << "]";
        return os.str();
    }

    template<typename T, typename U>
    std::string to_debug_string(const std::pair<T,U> &x){
        return to_debug_string(x.first)+"\t"+to_debug_string(x.second);
    }
    
    template <size_t number>
    struct tuple_printer{
        template <typename TupleType>
        static void print(std::ostream & os,TupleType const & TP) noexcept{
            os << std::get<std::tuple_size<TupleType>::value - number>(TP) << ", ";
            tuple_printer<number-1>::print(os,TP);
        }
    };
    template <>
    struct tuple_printer<1>{
        template <typename TupleType>
        static void print(std::ostream & os,TupleType const & TP) noexcept{
            os << std::get<std::tuple_size<TupleType>::value - 1>(TP);
        }
    };
    template <>
    struct tuple_printer<0>{
        template <typename TupleType>
        static void print(std::ostream & os,TupleType const & TP) noexcept{}
    };
    

    template<typename...Args>
    std::string to_debug_string(std::tuple<Args...> const & TP){
        std::ostringstream os;
        os << "(";
        tuple_printer<std::tuple_size<std::tuple<Args...>>::value>::print(os,TP);
        os << ")";
        return os.str();
    }

    template <typename Equation,typename Expectation>
    void __test_impl__(std::string const &test_name,Equation const& eq_res,Expectation const & exp_res){
        if(eq_res == exp_res){
            std::cout << "OK, TEST(" + test_name + "): " + to_debug_string(eq_res) + " = " + to_debug_string(exp_res) <<std::endl;
        }else{
            std::cout << "ERROR, TEST(" + test_name + "): " + to_debug_string(eq_res) + " != " + to_debug_string(exp_res) <<std::endl;
        }
    }
};


#define SVAR(x) (std::string(#x) + std::string(" = ") + debugdefs::to_debug_string(x))
#define PVAR(x) std::cout << SVAR(x) <<std::endl

#define PDEL() (std::cout << "-----------------------------------------------" <<std::endl)
#define SCOMPARE(x,y) (SVAR(x) + " vs " + SVAR(y))
#define COMPARE(x,y) std::cout << SCOMPARE(x,y) <<std::endl;

#define DEBUG std::cout << __PRETTY_FUNCTION__ <<std::endl;

#define TEST(equation,expectation) (debugdefs::__test_impl__(SVAR(equation),\
                                                debugdefs::to_debug_string(equation), \
                                               debugdefs::to_debug_string(expectation )))

template <typename T>
std::string TypeString(){
    std::string fname = __PRETTY_FUNCTION__ ;
    return fname.substr(35,fname.size()-84);
}
#define TypeToString(type) TypeString<type>()

template <typename EnumClass>
struct EnNameStr{
	template <EnumClass em>
	static std::string _str(){
		std::string enum_name = __PRETTY_FUNCTION__ ;
		return  enum_name.substr(69,enum_name.size()-135);
	}
};
#define ValueToString(V) EnNameStr<decltype(V)>::_str<V>()

void print(){
    std::cout <<std::endl;
}
template <typename T>
void print(T data){
    std::cout << data <<std::endl;
}

template <typename ...Args, typename T>
void print(T data,Args...args){
    std::cout << data;
    print(args...);
}
template <typename U,typename V>
std::ostream & operator << (std::ostream & os, const std::pair<U,V> & P){
    os << "pair(" << P.first << ", " << P.second << ")";
    return os;
}

template<typename...Args>
std::ostream & operator << (std::ostream & os,const std::tuple<Args...> & TP){
    os << debugdefs::to_debug_string(TP);
    return os;
}
template<typename...Args>
std::ostream & operator << (std::ostream & os,const std::vector<Args...> & TP){
    std::ostringstream out;
    //TP = 1+"kl;";
    out << "[";
    if(TP.size()){
        out << TP[0];
    }
    for(size_t i=1;i<TP.size();++i){
        out << "," << TP[i];  
    }out << "]";
    return os << out.str();
}

#endif