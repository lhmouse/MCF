#include <MCF/StdMCF.hpp>
#include <MCF/Components/EventDriver.hpp>
#include <cstdio>

bool foo(int id, std::uintptr_t){
	std::printf("id = %d\n", id);
	return false;
}

unsigned int MCFMain(){
	auto h0 = MCF::RegisterEventHandler(1, std::bind(foo, 0, std::placeholders::_1));
	auto h1 = MCF::RegisterEventHandler(1, std::bind(foo, 1, std::placeholders::_1));
	auto h2 = MCF::RegisterEventHandler(1, std::bind(foo, 2, std::placeholders::_1));
	auto h3 = MCF::RegisterEventHandler(1, std::bind(foo, 3, std::placeholders::_1));
	auto h4 = MCF::RegisterEventHandler(2, std::bind(foo, 4, std::placeholders::_1));
	auto h5 = MCF::RegisterEventHandler(2, std::bind(foo, 5, std::placeholders::_1));

	MCF::RaiseEvent(1, 12345);
	MCF::RaiseEvent(2, 54321);

	h1.Reset();
	h3.Reset();

	MCF::RaiseEvent(1, 6789);

	return 0;
}
