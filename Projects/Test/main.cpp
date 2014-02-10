#include <MCFCRT/MCFCRT.h>
#include <MCF/Components/EventDriver.hpp>
#include <cstdio>

bool foo(int id, std::uintptr_t ctx) noexcept {
	std::printf("foo(%d, %zu)\n", id, ctx);
	return false;
}

unsigned int MCFMain(){
	auto h0 = MCF::RegisterEventHandler(1, std::bind(&foo, 0, std::placeholders::_1));
	auto h1 = MCF::RegisterEventHandler(1, std::bind(&foo, 1, std::placeholders::_1));
	auto h2 = MCF::RegisterEventHandler(1, std::bind(&foo, 2, std::placeholders::_1));

	MCF::RaiseEvent(1, 12345);
	std::puts("------");

	h1.Reset();
	MCF::RaiseEvent(1, 12345);
	std::puts("------");

	h2.Reset();
	h0.Reset();
	MCF::RaiseEvent(1, 12345);

	return 0;
}
