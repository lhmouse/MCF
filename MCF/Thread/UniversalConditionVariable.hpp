// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include "ConditionVariable.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class UniversalConditionVariable : NONCOPYABLE {
private:
	Mutex $vMutex;
	ConditionVariable $vDelegate;

public:
	explicit UniversalConditionVariable(std::size_t uSpinCount = 0x100)
		: $vMutex(uSpinCount), $vDelegate($vMutex)
	{
	}

public:
	bool Wait(UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		$vMutex.Lock();
		vLock.Unlock();

		const bool bResult = $vDelegate.Wait(u64MilliSeconds);

		vLock.Lock();
		$vMutex.Unlock();

		return bResult;
	}
	void Wait(UniqueLockTemplateBase &vLock) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		$vMutex.Lock();
		vLock.Unlock();

		$vDelegate.Wait();

		vLock.Lock();
		$vMutex.Unlock();
	}
	void Signal(std::size_t uMaxCount) noexcept {
		$vDelegate.Signal(uMaxCount);
	}
	void Broadcast() noexcept {
		$vDelegate.Broadcast();
	}
};

}

#endif
