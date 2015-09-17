// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include "ConditionVariable.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class UniversalConditionVariable : NONCOPYABLE {
private:
	Mutex x_mtxGuard;
	ConditionVariable x_condDelegator;

public:
	explicit UniversalConditionVariable(std::size_t uSpinCount = 0x100)
		: x_mtxGuard(uSpinCount), x_condDelegator()
	{
	}

public:
	bool Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		x_mtxGuard.Lock();
		vLock.Unlock();
		const bool bTakenOver = x_condDelegator.Wait(x_mtxGuard, u64MilliSeconds);

		x_mtxGuard.Unlock();
		vLock.Lock();
		return bTakenOver;
	}
	void Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		x_mtxGuard.Lock();
		vLock.Unlock();
		x_condDelegator.Wait(x_mtxGuard);

		x_mtxGuard.Unlock();
		vLock.Lock();
	}
	void Signal(std::size_t uMaxCount = 1) noexcept {
		x_condDelegator.Signal(uMaxCount);
	}
	void Broadcast() noexcept {
		x_condDelegator.Broadcast();
	}
};

}

#endif
