#include <MCFCRT/MCFCRT.h>
#include <MCF/Core/CriticalSection.hpp>
#include <MCF/Core/ReadWriteLock.hpp>
#include <MCF/Core/Thread.hpp>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

MCF::CriticalSection cs;
MCF::ReadWriteLock rwl;

void reader(int i){
	::Sleep(100);
	READ_LOCK_SCOPE(rwl){
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "reader   %d - thread %lu - entered\n", i, ::GetCurrentThreadId());
		}
		READ_LOCK_SCOPE(rwl){
			READ_LOCK_SCOPE(rwl){
				CRITICAL_SECTION_SCOPE(cs){
					std::fprintf(stderr, "reader   %d - thread %lu - reading\n", i, ::GetCurrentThreadId());
				}
				::Sleep(100);
			}
		}
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "reader   %d - thread %lu - left\n", i, ::GetCurrentThreadId());
		}
	}
}

void writer(int i){
	::Sleep(99);
	WRITE_LOCK_SCOPE(rwl){
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "  writer %d - thread %lu - entered\n", i, ::GetCurrentThreadId());
		}
		WRITE_LOCK_SCOPE(rwl){
			WRITE_LOCK_SCOPE(rwl){
				CRITICAL_SECTION_SCOPE(cs){
					std::fprintf(stderr, "  writer %d - thread %lu - writing\n", i, ::GetCurrentThreadId());
				}
				::Sleep(100);
			}
		}
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "  writer %d - thread %lu - left\n", i, ::GetCurrentThreadId());
		}
	}
}

unsigned int MCFMain(){
	MCF::Thread threads[32];
	unsigned i = 0;
	while(i < 24){
		threads[i].Start(std::bind(reader, i));
		++i;
	}
	while(i < 32){
		threads[i].Start(std::bind(writer, i));
		++i;
	}
	return 0;
}
