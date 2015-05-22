#include <MCF/StdMCF.hpp>
#include <MCF/Core/Uuid.hpp>
#include <iostream>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto uuid = Uuid::Generate();

	char temp[36];
	uuid.Print(temp);
	std::endl(std::cout.write(temp, sizeof(temp)));

	return 0;
}
