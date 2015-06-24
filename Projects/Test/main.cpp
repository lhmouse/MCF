#include <MCF/StdMCF.hpp>
#include <cstdio>

#include <MCFCRT/env/thread.h>

using namespace MCF;

extern "C" unsigned int MCFMain() noexcept {
	const auto key = ::MCF_CRT_TlsAllocKey([](std::intptr_t val){
		auto p = (int *)val;
		if(p){
			std::printf("destructing %d\n", *p);
			delete p;
		}
	});

	::MCF_CRT_TlsReset(key, (std::intptr_t)new int(3));
	::MCF_CRT_TlsReset(key, (std::intptr_t)new int(4));

	auto thrd = ::MCF_CRT_CreateThread(
		[](std::intptr_t k) -> unsigned {
			const auto key = (void *)k;
			::MCF_CRT_TlsReset(key, (std::intptr_t)new int(1));
			::MCF_CRT_TlsReset(key, (std::intptr_t)new int(2));

			try {
				std::puts("about to throw...");
				throw std::out_of_range("test out_of_range in thread");
			} catch(std::exception &e){
				std::printf("exception caught: what = %s\n", e.what());
			}

			return 0;
		},
		(std::intptr_t)key, false, nullptr);
	::WaitForSingleObject((HANDLE)thrd, INFINITE);

	try {
		std::puts("about to throw...");
		throw std::out_of_range("test out_of_range");
	} catch(std::exception &e){
		std::printf("exception caught: what = %s\n", e.what());
	}

	return 0;
}

