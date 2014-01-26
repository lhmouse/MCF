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
	::Sleep(10);
	READ_LOCK_SCOPE(rwl){
		::Sleep(10);
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "reader    %2d - thread %5lu - entered\n", i, ::GetCurrentThreadId());
		}
		::Sleep(10);
		READ_LOCK_SCOPE(rwl){
			::Sleep(10);
			READ_LOCK_SCOPE(rwl){
				::Sleep(10);
				READ_LOCK_SCOPE(rwl){
					::Sleep(100);
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, " reader   %2d - thread %5lu - reading\n", i, ::GetCurrentThreadId());
					}
					::Sleep(100);
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, "  reader  %2d - thread %5lu - done\n", i, ::GetCurrentThreadId());
					}
				}
			}
		}
		::Sleep(10);
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "   reader %2d - thread %5lu - left\n", i, ::GetCurrentThreadId());
		}
	}
}

void writer(int i){
	::Sleep(9);

	auto l1 = rwl.GetWriteLock();
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "writer    %2d - thread %5lu - entered\n", i, ::GetCurrentThreadId());
		}
		::Sleep(10);
		auto l2 = rwl.GetReadLock();
			::Sleep(10);
			auto l3 = rwl.GetWriteLock();
				::Sleep(10);
				auto l4 = rwl.GetReadLock();
					::Sleep(100);
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, " writer   %2d - thread %5lu - writing\n", i, ::GetCurrentThreadId());
					}
					::Sleep(100);
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, "  writer  %2d - thread %5lu - done\n", i, ::GetCurrentThreadId());
					}
				l1.reset();
			l2.reset();
		l3.reset();
		::Sleep(10);
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "   writer %2d - thread %5lu - left\n", i, ::GetCurrentThreadId());
		}
	l4.reset();
}

void foo(){
	READ_LOCK_SCOPE(rwl){
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "foo          - thread %5lu - entered ********************\n",	 ::GetCurrentThreadId());
		}
		::Sleep(100);
		WRITE_LOCK_SCOPE(rwl){
			READ_LOCK_SCOPE(rwl){
				READ_LOCK_SCOPE(rwl){
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, " foo         - thread %5lu - reading ********************\n", ::GetCurrentThreadId());
					}
					::Sleep(1000);
					CRITICAL_SECTION_SCOPE(cs){
						std::fprintf(stderr, "  foo        - thread %5lu - done ********************\n", ::GetCurrentThreadId());
					}
				}
			}
		}
		CRITICAL_SECTION_SCOPE(cs){
			std::fprintf(stderr, "   foo       - thread %5lu - left ********************\n", ::GetCurrentThreadId());
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
	MCF::Thread spec;
	spec.Start(foo);
	return 0;
}
