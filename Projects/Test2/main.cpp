#include <MCF/StdMCF.hpp>
#include <MCF/Serialization/Serdes_tuple.hpp>
#include <iostream>
using namespace MCF;

typedef
	std::tuple<int, double, char>
	TestType;

extern "C" unsigned int MCFMain() noexcept {
	StreamBuffer buf;
	TestType t1 = std::make_tuple(12, 34.5, '6'), t2;
	Serialize(buf, t1);
	Deserialize(t2, buf);
	std::cout <<std::get<0>(t2) <<", " <<std::get<1>(t2) <<", " <<std::get<2>(t2);
	return 0;
}
