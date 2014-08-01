#include <MCF/StdMCF.hpp>
#include <MCF/Serialization/VarIntEx.hpp>
#include <iostream>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	VarIntEx<unsigned int> v1(0x12345678), v2;
	unsigned char buf[12];
	auto w = buf;
	v1.Serialize(w);
	auto r = buf;
	v2.Deserialize(r, w);
	std::cout <<std::hex <<v2.Get() <<std::endl;
	return 0;
}
