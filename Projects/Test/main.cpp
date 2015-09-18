#include <MCF/Core/Thunk.hpp>
#include <MCF/Containers/RingQueue.hpp>
#include <cstdio>

extern "C" unsigned MCFMain(){
	MCF::RingQueue<MCF::ThunkPtr> thunks;
	try {
		static char data[100];
		for(;;){
			thunks.Push(MCF::CreateThunk(data, sizeof(data)));
		}
	} catch(std::bad_alloc&e){
		std::printf("bad_alloc: allocated %zu thunks\n", thunks.GetSize());
	}
	return 0;
}
