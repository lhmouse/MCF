#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/Mutex.hpp>
#include <MCF/Core/Thread.hpp>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

MCF::Mutex mtx;

void proc(){
	::Sleep(100);
	MUTEX_SCOPE(mtx){
		std::fprintf(stderr, "thread %lu - entered\n", ::GetCurrentThreadId());
		::Sleep(100);
		std::fprintf(stderr, "thread %lu - left\n", ::GetCurrentThreadId());
	}
}

unsigned int MCFMain(){
	MCF::Thread threads[32];
	for(auto &thrd : threads){
		thrd.Start(proc);
	}
	return 0;
}
