#include <iostream>
#include "../include/grob/point.hpp"
#include "debug_defs.hpp"
int main(void){
	grob::Point<grob::Point<int,grob::Point<float,double>>,std::string> p(13,2.55,3.45,"str");
	//p.rget<2>() = 2.333;
	std::cout<< p <<std::endl;
	std::cout << p.rget<0>() << std::endl;
	std::cout << p.rget<1>() << std::endl;
	std::cout << p.rget<2>() << std::endl;
	std::cout << p.rget<3>() << std::endl;
	std::cout << decltype(p)::rget_index<0>::value << std::endl;
	std::cout << decltype(p)::rget_index<1>::value << std::endl;
	
	std::cout << "get item" << std::endl;
	std::cout << decltype(p)::rget_item<0>::value << std::endl;
	std::cout << decltype(p)::rget_item<1>::value << std::endl;
	std::cout << decltype(p)::rget_item<2>::value << std::endl;
	std::cout << decltype(p)::rget_item<3>::value << std::endl;
	
	//std::cout << decltype(p)::rget_index<2>::value << std::endl;
	
	auto P1 = grob::Rect<float>(0,1);
	auto P2 = grob::Rect<float>(0.5,3);

	auto IP = grob::intersect(grob::make_point(P1,P2),grob::make_point(P2,P1));
	PVAR(IP);
	print(TypeToString(decltype(IP)));
	return 0;
}