#include <iostream>
#include "../src/container_shift.hpp"
#include "../src/grid.hpp"
#include "debug_defs.hpp"
#include <vector>
#include <array>
#include "../src/multigrid.hpp"
#include "../src/csv_io.hpp"
#include "../src/grid_objects.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

int main(void){
	
	auto F = grob::make_function_f(
		grob::mesh_grids(grob::GridUniform<double>(0,1,5),
						grob::GridUniform<double>(0,1,5)),
						[](double x,double y){return x+y;});
	std::cout << "print F" << std::endl;
	grob::as_csv(F).save(std::cout);

	auto F1 = grob::make_function_f(
		grob::mesh_grids(grob::mesh_grids(grob::GridUniform<double>(0,1,5),
						grob::GridUniform<double>(0,1,5)),grob::GridUniform<double>(0,1,5)),
						[](double x,double y,double z){return x+y+z;});
	std::cout << "print F1" << std::endl;
	grob::as_csv(F1).save(std::cout);


	auto SophGrid = grob::make_grid_f(grob::GridUniform<double>(0,1,5),[](size_t i){
						return grob::GridUniform<double>(-0.2*i,1+0.2*i,5);
					});
	
	auto G = grob::make_function_f(SophGrid,
						[](double x,double y){return x+y;});
	
	std::cout << "print G" << std::endl;
	grob::as_csv(G).save(std::cout);

	auto F_recover = grob::make_function_f(F.Grid,G);

	PVAR(G(0,0));
	PVAR(G(0,0.25));
	PVAR(G(0.2,0.25));
	PVAR(G(0,0.25));

	std::cout << "print F recover" << std::endl;
	grob::as_csv(F_recover).save(std::cout);


	return 0;
}