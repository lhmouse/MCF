#include <MCF/StdMCF.hpp>
#include <MCF/Serialization/Serdes_bitset.hpp>
#include <iostream>
using namespace MCF;

typedef
	std::bitset<64>
	TestType;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	TestType t1(0x123456789ABCDEF0), t2;
	buf <<t1;
	buf >>t2;
	std::cout <<t1 <<std::endl;
	std::cout <<t2 <<std::endl;
	return 0;
}
