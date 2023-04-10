#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP
#include <string>
#include <iomanip>
#include "object_serialization.hpp"

//#include "../tests/debug_defs.hpp"

namespace stools{
    struct BinaryWriter:std::ostream{
        typedef std::ostream Base;
        using Base::Base;
        template <typename T>
        void write(const T & x){
            Base::write(&x,sizeof(T));
        }
    };
    struct BinaryReader:std::istream{
        typedef std::istream Base;
        using Base::Base;
        template <typename T>
        void read(T & x){
            Base::read(&x,sizeof(T));
        }
    };
    struct SequenceWriter:std::ostream{
        typedef std::ostream Base;
        using Base::Base;
        template <typename T>
        void write(const T & x){
            Base & stream = *this;
            stream << (x) << " ";
        }
    };
    struct SequenceReader:std::istream{
        typedef std::istream Base;
        using Base::Base;
        template <typename T>
        void read(T & x){
            Base & stream = *this;
            stream >> x;
        }
    };

    struct SerializatorJson{
        
        std::ostream * stream;
        size_t deep = 0;
        SerializatorJson(std::ostream & stream):stream(&stream),deep(0){}
        SerializatorJson(){}
        void setStream(std::ostream & new_stream){stream = &new_stream;deep = 0;}
        
        struct Object{};

        template <typename T>
        Object MakePrimitive(const T & x){
            //using namespace print_vector;
            (*stream) <<(x);
            return Object();
        }
        Object MakePrimitive(const std::string & s){
            (*stream) <<std::quoted(s);
            return Object();
        };

        template <typename KeyFunctype,typename ValueFunctype>
        Object MakeDict(size_t property_num,KeyFunctype &&keys,ValueFunctype && values){
            (*stream) << "{";
            if(property_num)
                (*stream) << "\n";
            ++deep;
            for(size_t i=0;i<property_num;++i){
                if(i)
                   (*stream) << ",\n";
                
                (*stream) <<std::string(deep*2,' ')<<std::quoted(keys(i)) << " : ";
                values(i);
            }
            --deep;
            if(property_num)
                (*stream) << "\n";
            (*stream) <<std::string(deep*2,' ')<<"}";
            
            return Object();
        }

        template <typename ValueFunctype>
        Object MakeArray(size_t size,ValueFunctype && values){
            (*stream) << "[";
            for(size_t i=0;i<size;++i){
                if(i)
                   (*stream) << ", ";
                values(i);
            }
            (*stream) <<"]";
            return Object();
        }
        
    };

    template <typename ptree_type>
    struct PtreeSerializator{
        template <typename T>
        static ptree_type MakePrimitive(const T & x){
            ptree_type p;
            p.put("",x);
            return p;
        }
        template <typename KeyFunctype,typename ValueFunctype>
        static ptree_type MakeDict(size_t property_num,KeyFunctype &&keys,ValueFunctype && values){
            ptree_type p;
            for(size_t i=0;i<property_num;++i){
                p.add_child(keys(i),values(i));
            }
            return p;
        }

        template <typename ValueFunctype>
        static ptree_type MakeArray(size_t size,ValueFunctype && values){
            ptree_type p;
            for(size_t i=0;i<size;++i){
                p.push_back(std::make_pair("",values(i)));
            }
            return p;
        }


        template <typename T>
        static void GetPrimitive(ptree_type const& p, T & x){
            x = p.template get<T>("");
        }

        template <typename string_type>
        static ptree_type GetProperty(ptree_type const&p,string_type && str){
            return p.get_child(str);
        }

        template <typename _ptree_type>
        static auto Begin(_ptree_type &&p){
            return p.begin();
        }
        template <typename _ptree_type>
        static auto End(_ptree_type &&p){
            return p.end();
        }
        template <typename iter_type>
        auto GetKey(iter_type &&  it){
            return it->first;
        }
        template <typename iter_type>
        auto GetValue(iter_type && it){
            return it->second;
        }
    };

};

#endif //SERIALIZATION_HPP