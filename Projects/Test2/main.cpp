#include <MCF/StdMCF.hpp>
#include <MCF/Thread/MonitorPtr.hpp>
#include <cstdio>
using namespace MCF;

struct foo {
	void meow(){
		std::puts("meow");
	}
};

extern "C" unsigned int MCFMain() noexcept {
	MonitorPtr<foo> mp;
//	MonitorPtr<std::unique_ptr<foo>> mp(std::make_unique<foo>());
	mp->meow();

	return 0;
}
