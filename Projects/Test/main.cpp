#include <MCF/StdMCF.hpp>
#include <MCF/Containers/Vector.hpp>
#include <MCF/Utilities/Thunk.hpp>
#include <MCF/Core/Random.hpp>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	static const char data[4096] = { };
	auto p = MCF::CreateThunk(data, MCF::GetRandomUint32() % sizeof(data));
	return 0;
}
