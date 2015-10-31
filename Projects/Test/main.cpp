#include <MCF/StdMCF.hpp>
#include <MCF/Core/Matrix.hpp>

using namespace MCF;

Matrix<unsigned, 2, 4> a = {{{ {{1,2,3,4}},{{5,6,7,8}} }}};
Matrix<unsigned, 2, 4> b = {{{ {{7,8,9}},{{1,2,3}} }}};
Matrix<unsigned, 2, 4> c;

extern "C" unsigned MCFMain(){
	c = a + b;

	for(unsigned y = 0; y < c.GetRowCount(); ++y){
		std::printf("[ ");
		for(unsigned x = 0; x < c.GetColumnCount(); ++x){
			std::printf("%2u ", c[y][x]);
		}
		std::printf("]\n");
	}

	return 0;
}
