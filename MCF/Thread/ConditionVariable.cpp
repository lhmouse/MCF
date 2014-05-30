// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ConditionVariable.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

// http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
// 因为 Semaphore 现在维护一个大体上 FIFO 的顺序，我们就没必要操心了。

namespace {

class ConditionVariableDelegate : CONCRETE(ConditionVariable) {
private:
	volatile std::size_t xm_uWaiterCount;
	Impl::UniqueWinHandle xm_hSemaphore;

public:
	ConditionVariableDelegate() noexcept
		: xm_uWaiterCount(0)
	{
		xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, 0, 0x7FFFFFFF, nullptr));
		if(!xm_hSemaphore){
			MCF_THROW(::GetLastError(), L"CreateSemaphoreW() 失败。"_WSO);
		}
	}

public:
	bool WaitTimeout(LockRaiiTemplateBase &vLockRaiiTemplate, unsigned long long ullMilliSeconds) noexcept {
		__atomic_add_fetch(&xm_uWaiterCount, 1, __ATOMIC_RELAXED);
		const auto uReentrantCount = vLockRaiiTemplate.UnlockAll();
		ASSERT(uReentrantCount != 0);

		const bool bResult = WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				return ::WaitForSingleObject(xm_hSemaphore.Get(), dwRemaining) != WAIT_TIMEOUT;
			},
			ullMilliSeconds
		);
		vLockRaiiTemplate.Lock(uReentrantCount);
		return bResult;
	}
	void Signal(std::size_t uCount) noexcept {
		std::size_t uOldWaiterCount = __atomic_load_n(&xm_uWaiterCount, __ATOMIC_RELAXED);
		std::size_t uCountToSignal;
		for(;;){
			uCountToSignal = std::min(uCount, uOldWaiterCount);
			if(uCountToSignal == 0){
				break;
			}
			if(EXPECT_NOT(__atomic_compare_exchange_n(
				&xm_uWaiterCount, &uOldWaiterCount, uOldWaiterCount - uCountToSignal,
				false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)
			)){
				::ReleaseSemaphore(xm_hSemaphore.Get(), (long)uCountToSignal, nullptr);
				break;
			}
		}
	}
	void Broadcast() noexcept {
		::ReleaseSemaphore(xm_hSemaphore.Get(), (long)__atomic_exchange_n(&xm_uWaiterCount, 0, __ATOMIC_RELAXED), nullptr);
	}
};

}

// 静态成员函数。
std::unique_ptr<ConditionVariable> ConditionVariable::Create(){
	return std::make_unique<ConditionVariableDelegate>();
}

// 其他非静态成员函数。
bool ConditionVariable::WaitTimeout(LockRaiiTemplateBase &vLockRaiiTemplate, unsigned long long ullMilliSeconds) noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	return static_cast<ConditionVariableDelegate *>(this)->WaitTimeout(vLockRaiiTemplate, ullMilliSeconds);
}
void ConditionVariable::Wait(LockRaiiTemplateBase &vLockRaiiTemplate) noexcept {
	WaitTimeout(vLockRaiiTemplate, WAIT_FOREVER);
}
void ConditionVariable::Signal(std::size_t uCount) noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	static_cast<ConditionVariableDelegate *>(this)->Signal(uCount);
}
void ConditionVariable::Broadcast() noexcept {
	ASSERT(dynamic_cast<ConditionVariableDelegate *>(this));

	static_cast<ConditionVariableDelegate *>(this)->Broadcast();
}
