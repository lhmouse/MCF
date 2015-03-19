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

namespace MCF {

class UniversalConditionVariable : NONCOPYABLE {
private:
	Mutex x_vMutex;
	ConditionVariable x_vDelegate;

public:
	explicit UniversalConditionVariable(std::size_t uSpinCount = 0x400)
		: x_vMutex(uSpinCount), x_vDelegate(x_vMutex)
	{
	}

public:
	bool Wait(UniqueLockTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		x_vMutex.Lock();
		vLock.Unlock();

		const bool bResult = x_vDelegate.Wait(ullMilliSeconds);

		vLock.Lock();
		x_vMutex.Unlock();
		return bResult;
	}
	void Wait(UniqueLockTemplateBase &vLock) noexcept {
		ASSERT(vLock.GetLockCount() == 1);

		x_vMutex.Lock();
		vLock.Unlock();

		x_vDelegate.Wait();

		vLock.Lock();
		x_vMutex.Unlock();
	}
	void Signal(std::size_t uMaxCount) noexcept {
		x_vDelegate.Signal(uMaxCount);
	}
	void Broadcast() noexcept {
		x_vDelegate.Broadcast();
	}
};

}

#endif
