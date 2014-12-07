// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

// 构造函数和析构函数。
ConditionVariable::ConditionVariable()
	: xm_vSemaphore(0, nullptr), xm_ulWaiting(0)
{
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(LockRaiiTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept {
	__atomic_add_fetch(&xm_ulWaiting, 1, __ATOMIC_RELAXED);
	vLock.Unlock();
	ASSERT(!vLock.IsLocking());

	const bool bResult = xm_vSemaphore.Wait(ullMilliSeconds);
	vLock.Lock();
	return bResult;
}
void ConditionVariable::Wait(LockRaiiTemplateBase &vLock) noexcept {
	__atomic_add_fetch(&xm_ulWaiting, 1, __ATOMIC_RELAXED);
	vLock.Unlock();
	ASSERT(!vLock.IsLocking());

	xm_vSemaphore.Wait();
	vLock.Lock();
}
void ConditionVariable::Signal(unsigned long ulMaxCount) noexcept {
	auto ulWaiting = __atomic_load_n(&xm_ulWaiting, __ATOMIC_RELAXED);
	unsigned long ulToSignal;
	for(;;){
		ulToSignal = Min(ulWaiting, ulMaxCount);
		if(EXPECT_NOT(__atomic_compare_exchange_n(
			&xm_ulWaiting, &ulWaiting, ulWaiting - ulToSignal, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)))
		{
			break;
		}
	}
	if(ulToSignal != 0){
		xm_vSemaphore.Post(ulToSignal);
	}
}
void ConditionVariable::SignalAll() noexcept {
	const auto ulToSignal = __atomic_exchange_n(&xm_ulWaiting, 0, __ATOMIC_RELAXED);
	if(ulToSignal != 0){
		xm_vSemaphore.Post(ulToSignal);
	}
}
