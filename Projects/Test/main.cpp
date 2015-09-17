#include <MCF/Thread/Thread.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <MCF/Containers/RingQueue.hpp>
#include <MCFCRT/env/mcfwin.h>
#include <iostream>
#include <string>

using namespace MCF;

Mutex queue_mutex;
ConditionVariable queue_cv;
RingQueue<std::string> queue;

Mutex cout_mutex;

extern "C" unsigned MCFMain(){
	auto thread = Thread::Create([]{
		for(;;){
			Mutex::UniqueLock lock(queue_mutex);
			while(queue.IsEmpty()){
				{ const Mutex::UniqueLock cout_lock(cout_mutex);
				  std::cout <<"Consumer is waiting for data..." <<std::endl; }
				queue_cv.Wait(queue_mutex);
			}
			auto str = *std::move(queue.GetFirst());
			queue.Shift();
			lock.Unlock();

			if(str.empty()){
				break;
			}

			::Sleep(100);
			{ const Mutex::UniqueLock cout_lock(cout_mutex);
			  std::cout <<"Consumed: " <<str <<std::endl; }
		}
	});

	for(unsigned i = 0; i < 10; ++i){
		::Sleep(1000);

		for(unsigned j = 0; j < 5; ++j){
			char str[256];
			std::sprintf(str, "string %u ---------------------------", i * 100 + j);
			{ const Mutex::UniqueLock cout_lock(cout_mutex);
			  std::cout <<"Produced: " <<str <<std::endl; }

			Mutex::UniqueLock lock(queue_mutex);
			queue.Push(str);
			queue_cv.Signal();
		}
	}

	{ Mutex::UniqueLock lock(queue_mutex);
	  queue.Push(); }
	thread->Join();

	return 0;
}
