#include <MCF/StdMCF.hpp>
#include <MCF/Utilities/Invoke.hpp>
#include <string>
#include <cstdio>

using namespace MCF;

struct foo {
	void bark() const && {
	};
};

extern "C" unsigned int MCFMain() noexcept {
	auto fn = &foo::bark;
	foo f;
	Invoke(fn, std::move(f));
	return 0;
}
