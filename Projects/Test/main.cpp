#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	MCF::NarrowString str;
	for(char c = ' '; c < '~'; ++c){
		str += c;
	}
	auto f = static_cast<const char * (MCF::NarrowString::*)() const noexcept>(MCF::NarrowString::GetStr);
	__builtin_puts((str.*f)());
	return 0;
}
