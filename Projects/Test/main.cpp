#include <MCF/StdMCF.hpp>
#include <MCF/Core/Matrix.hpp>

using namespace MCF;

extern "C" unsigned MCFMain(){
	Matrix<unsigned, 2, 3> a = {{{ {1,2,3},{4,5,6} }}};
	Matrix<unsigned, 3, 2> b = {{{ {1,4},{2,5},{3,6} }}};
	Matrix<unsigned, 2, 2> c = a * b;

	for(unsigned y = 0; y < c.GetRowCount(); ++y){
		std::printf("[ ");
		for(unsigned x = 0; x < c.GetColumnCount(); ++x){
			std::printf("%02u ", c[y][x]);
		}
		std::printf("]\n");
	}

	return 0;
}
