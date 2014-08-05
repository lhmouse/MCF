#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <MCF/Core/String.hpp>
#include <MCF/Serialization/VarIntEx.hpp>
using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	try {
		unsigned char temp[9];

		VarIntEx<std::int32_t> v32(-0x8000);
		auto wp = temp;
		v32.Serialize(wp);

		VarIntEx<std::int16_t> v16;
		auto rp = temp;
		v16.Deserialize(rp, wp - temp);

		std::printf("deserialized %d\n", v16.Get());
	} catch(Exception &e){
		std::printf("exception %s\n", AnsiString(e.m_wcsMessage).GetCStr());
	}
	return 0;
}
