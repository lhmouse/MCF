// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "WinHandle.inl"
#include "../Core/Exception.hpp"
#include "../Core/StringObserver.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"
#include <climits>
using namespace MCF;

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

namespace {

class ConditionVariableDelegate : CONCRETE(ConditionVariable) {
private:
	Impl::UniqueWinHandle xm_hSemaphore;
	volatile std::size_t xm_uWaiterCount;

public:
	ConditionVariableDelegate()
		: xm_uWaiterCount(0)
	{
		if(!xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, 0, LONG_MAX, nullptr))){
			DEBUG_THROW(SystemError, "CreateSemaphoreW");
		}
	}

public:
	bool WaitTimeout(LockRaiiTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept {
		__atomic_add_fetch(&xm_uWaiterCount, 1, __ATOMIC_RELAXED);

		vLock.Unlock();
		ASSERT(!vLock.IsLocking());

		const bool bResult = WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), dwRemaining);
				if(dwResult == WAIT_FAILED){
					ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
				}
				return dwResult != WAIT_TIMEOUT;
			},
			ullMilliSeconds
		);
		vLock.Lock();
		return bResult;
	}
	void Signal(std::size_t uCount) noexcept {
		std::size_t uOldWaiterCount = __atomic_load_n(&xm_uWaiterCount, __ATOMIC_RELAXED);
		std::size_t uCountToSignal;
		for(;;){
			uCountToSignal = Min(uCount, uOldWaiterCount);
			if(EXPECT_NOT(__atomic_compare_exchange_n(
				&xm_uWaiterCount, &uOldWaiterCount, uOldWaiterCount - uCountToSignal,
				false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)))
			{
				break;
			}
		}
		if(uCountToSignal != 0){
			if(!::ReleaseSemaphore(xm_hSemaphore.Get(), (long)uCountToSignal, nullptr)){
				ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
			}
		}
	}
	void Broadcast() noexcept {
		if(!::ReleaseSemaphore(xm_hSemaphore.Get(),
			(long)__atomic_exchange_n(&xm_uWaiterCount, 0, __ATOMIC_RELAXED), nullptr))
		{
			ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<ConditionVariable> ConditionVariable::Create(){
	return std::make_unique<ConditionVariableDelegate>();
}

// 其他非静态成员函数。
bool ConditionVariable::WaitTimeout(LockRaiiTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	return static_cast<ConditionVariableDelegate *>(this)->WaitTimeout(vLock, ullMilliSeconds);
}
void ConditionVariable::Wait(LockRaiiTemplateBase &vLock) noexcept {
	WaitTimeout(vLock, WAIT_FOREVER);
}
void ConditionVariable::Signal(std::size_t uCount) noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	static_cast<ConditionVariableDelegate *>(this)->Signal(uCount);
}
void ConditionVariable::Broadcast() noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	static_cast<ConditionVariableDelegate *>(this)->Broadcast();
}
