#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Serialization/VarIntEx.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	std::printf("%s", "a"_nso.GetBegin());
	return 0;
}
