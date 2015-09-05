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
ConditionVariable::ConditionVariable(Mutex &vMutex)
	: $vMutex(vMutex), $uWaiting(0), $vSemaphore(0, nullptr)
{
}

// 其他非静态成员函数。
bool ConditionVariable::Wait(std::uint64_t u64MilliSeconds) noexcept {
	$uWaiting.Increment(kAtomicRelaxed);
	$vMutex.Unlock();

	const bool bResult = $vSemaphore.Wait(u64MilliSeconds);

	$vMutex.Lock();
	if(!bResult){
		$uWaiting.Decrement(kAtomicRelaxed);
	}
	return bResult;
}
void ConditionVariable::Wait() noexcept {
	$uWaiting.Increment(kAtomicRelaxed);
	$vMutex.Unlock();

	$vSemaphore.Wait();

	$vMutex.Lock();
}
void ConditionVariable::Signal(std::size_t uMaxCount) noexcept {
	std::size_t uToPost;
	std::size_t uOld = $uWaiting.Load(kAtomicRelaxed), uNew;
	do {
		if(uOld >= uMaxCount){
			uToPost = uMaxCount;
			uNew = uOld - uMaxCount;
		} else {
			uToPost = uOld;
			uNew = 0;
		}
	} while(!$uWaiting.CompareExchange(uOld, uNew, kAtomicRelaxed));

	if(uToPost != 0){
		$vSemaphore.Post(uToPost);
	}
}
void ConditionVariable::Broadcast() noexcept {
	const auto uToPost = $uWaiting.Exchange(0, kAtomicRelaxed);

	if(uToPost != 0){
		$vSemaphore.Post(uToPost);
	}
}

}
