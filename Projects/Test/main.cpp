#include <MCF/StdMCF.hpp>
#include <MCF/Thread/Mutex.hpp>
#include <MCF/Thread/ConditionVariable.hpp>
#include <cstdio>

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	MCF::Mutex m;
	MCF::ConditionVariable cv;
	auto l = m.GetLock();
	std::printf("locked = %d\n", l.IsLocking());
	auto s = cv.Wait(l, 0);
	std::printf("signaled = %d, locked = %d\n", s, l.IsLocking());
	s = cv.WaitOrAbandon(l, 0);
	std::printf("signaled = %d, locked = %d\n", s, l.IsLocking());
	return 0;
}
