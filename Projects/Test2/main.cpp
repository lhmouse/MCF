#include <MCF/StdMCF.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Serialization/Serdes.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto ws = L"12345678901234567890123456789012345678901234567890123456789012345"_ws;
	StreamBuffer buf;
	Serialize(buf, ws);
	std::printf("serialized size = %zu\n", buf.GetSize());
	AnsiString as;
	Deserialize(as, buf);
	std::puts(as.GetCStr());

	return 0;
}
