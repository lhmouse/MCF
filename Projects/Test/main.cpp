#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/Event.hpp>
#include <MCF/Containers/RingQueue.hpp>
#include <MCFCRT/env/mcfwin.h>
#include <iostream>
#include <string>

using namespace MCF;

Mutex queue_mutex;
Event sem(false);
RingQueue<std::string> queue;

Mutex cout_mutex;

extern "C" unsigned MCFMain(){
	auto thread = Thread::Create([]{
		unsigned delay = 0;

		for(;;){
			sem.Wait();
			Mutex::UniqueLock lock(queue_mutex);
			auto str = *std::move(queue.GetFirst());
			queue.Shift();
			if(queue.IsEmpty()){
				sem.Reset();
			}
			lock.Unlock();

			if(str.empty()){
				break;
			}

			::Sleep(delay);
			delay += 1;

			{ const Mutex::UniqueLock cout_lock(cout_mutex);
			  std::cout <<"         Consumed : " <<str <<std::endl; }
		}
	});

	unsigned delay = 1000;

	for(unsigned i = 0; i < 10; ++i){
		::Sleep(delay);
		if(delay > 0){
			delay -= 100;
		}

		for(unsigned j = 0; j < 10; ++j){
			char str[256];
			std::sprintf(str, "string %u ---------------------------", i * 100 + j);
			{ const Mutex::UniqueLock cout_lock(cout_mutex);
			  std::cout <<"Produced          : " <<str <<std::endl; }

			Mutex::UniqueLock lock(queue_mutex);
			queue.Push(str);
			sem.Set();
		}
	}

	{ Mutex::UniqueLock lock(queue_mutex);
	  queue.Push();
	  sem.Set(); }
	thread->Join();

	return 0;
}
