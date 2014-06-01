// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "Localization.hpp"
#include "JobScheduler.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Thread/CriticalSection.hpp"
#include <exception>
#include <cstdio>
using namespace MCFBuild;

extern "C" unsigned MCFMain()
try {
	int x = 0;
	JobScheduler sch;
	for(int i = 0; i < 100; ++i){
		sch.AddJob(
			[&, i]{
				std::printf("job %d begins\n", i);
				__atomic_add_fetch(&x, 1, __ATOMIC_RELAXED);
				std::printf("job %d ends\n", i);
			}
		);
	}
	sch.CommitAll(6);
	std::printf("x = %d\n", x);
	return 0;
} catch(std::exception &e){
	std::printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const MCF::Exception *>(&e);
	if(p){
		std::printf("  err  = %lu\n", p->m_ulErrorCode);
		std::printf("  desc = %s\n", MCF::AnsiString(MCF::GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		std::printf("  func = %s\n", p->m_pszFunction);
		std::printf("  line = %lu\n", p->m_ulLine);
		std::printf("  msg  = %s\n", MCF::AnsiString(FormatString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
