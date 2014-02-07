#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/Thread.hpp>
#include <MCF/Components/EventDriver.hpp>
#include <vector>
#include <cstdio>

void ThreadProc(){
	for(;;){
		std::puts("-----");
		MCF::RaiseEvent(1, 12345);
	}
}

unsigned int MCFMain(){
	MCF::Thread thread;
	thread.Start(&ThreadProc);

	const auto fn = [](int id, std::uintptr_t ctx){
		std::printf("handler, id = %d, ctx = %zu\n", id, ctx);
		return false;
	};

	std::vector<MCF::EventHandlerHolder> v;
	for(;;){
		const int ch = std::getchar();
		if(ch == 'z'){
			v.clear();
		} else {
			v.push_back(MCF::RegisterEventHandler(1, std::bind(fn, ch, std::placeholders::_1)));
		}
	}

	return 0;
}
