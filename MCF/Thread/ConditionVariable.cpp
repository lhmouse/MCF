// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "Mutex.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

// 构造函数和析构函数。
ConditionVariable::ConditionVariable()
	: x_uWaiting(0), x_vSemaphore(0, nullptr)
{
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	x_uWaiting.Increment(kAtomicRelaxed);
	vLock.Unlock();
	const bool bTakenOver = x_vSemaphore.Wait(u64MilliSeconds);

	vLock.Lock();
	if(!bTakenOver){
		x_uWaiting.Decrement(kAtomicRelaxed);
		return false;
	}
	return true;
}
void ConditionVariable::Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
	ASSERT(vLock.GetLockCount() == 1);

	x_uWaiting.Increment(kAtomicRelaxed);
	vLock.Unlock();
	x_vSemaphore.Wait();

	vLock.Lock();
}
void ConditionVariable::Signal(std::size_t uMaxCount) noexcept {
	std::size_t uToPost;
	std::size_t uOld = x_uWaiting.Load(kAtomicRelaxed), uNew;
	do {
		if(uOld >= uMaxCount){
			uToPost = uMaxCount;
			uNew = uOld - uMaxCount;
		} else {
			uToPost = uOld;
			uNew = 0;
		}
	} while(!x_uWaiting.CompareExchange(uOld, uNew, kAtomicRelaxed));

	if(uToPost != 0){
		x_vSemaphore.Post(uToPost);
	}
}
void ConditionVariable::Broadcast() noexcept {
	const auto uToPost = x_uWaiting.Exchange(0, kAtomicRelaxed);

	if(uToPost != 0){
		x_vSemaphore.Post(uToPost);
	}
}

}
