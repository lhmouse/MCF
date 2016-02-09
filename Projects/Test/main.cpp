#include <MCF/StdMCF.hpp>
#include <MCF/Core/Matrix.hpp>
#include <iostream>

extern "C" unsigned MCFCRT_Main(){
	MCF::Matrix<int, 2, 3> m1 = {{{ {1,2,3},{4,5,6}   }}};
	MCF::Matrix<int, 3, 2> m2 = {{{ {1,4},{2,5},{3,6} }}};
	auto m = m1 * m2;
	std::printf("m.rows = %zu, m.cols = %zu\n", m.kRows, m.kColumns);
	std::printf("[ %d %d ]\n", m[0][0], m[0][1]);
	std::printf("[ %d %d ]\n", m[1][0], m[1][1]);
	return 0;
}
