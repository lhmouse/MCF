#include <MCF/Core/String.hpp>

extern "C" unsigned _MCFCRT_Main() noexcept {
	MCF::NarrowString str;
	volatile auto f1 = static_cast<void (MCF::NarrowString::*)(char, std::size_t)>(&MCF::NarrowString::Append);
	for(char c = ' '; c < '~'; ++c){
		(str.*f1)(c, 1);
	}
	volatile auto f2 = static_cast<const char * (MCF::NarrowString::*)() const noexcept>(&MCF::NarrowString::GetStr);
	__builtin_puts((str.*f2)());
	return 0;
}
