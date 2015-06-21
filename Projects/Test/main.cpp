#include <MCF/StdMCF.hpp>
#include <MCF/Function/Bind.hpp>
#include <MCF/Core/String.hpp>
#include <cstdio>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	auto s = "hello world!"_u8s;
	auto fn = Bind([](auto &&s){ return static_cast<decltype(s)>(s); }, RefWrapper<Utf8String &&>(s));
	std::printf("fn() = %s\n", fn().GetStr());
	std::printf("s = %s\n", s.GetStr());
	return 0;
}
