// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class ConditionVariable : NONCOPYABLE {
private:
	Mutex x_mtxGuard;
	Atomic<std::uintptr_t> x_uControl;

public:
	constexpr ConditionVariable() noexcept
		: x_mtxGuard(), x_uControl(0)
	{
	}

public:
	bool Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept;
	void Wait(Mutex::UniqueLock &vLock) noexcept;

	void Signal() noexcept;
	void Broadcast() noexcept;

	bool Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept {
		ASSERT(vLock.IsLocking());

		std::size_t uLockCount;
		bool bTakenOver;
		{
			Mutex::UniqueLock vGuardLock(x_mtxGuard);
			uLockCount = vLock.X_UnlockAll();
			bTakenOver = Wait(vGuardLock, u64MilliSeconds);
		}
		vLock.X_RelockAll(uLockCount);
		return bTakenOver;
	}
	void Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept {
		ASSERT(vLock.IsLocking());

		std::size_t uLockCount;
		{
			Mutex::UniqueLock vGuardLock(x_mtxGuard);
			uLockCount = vLock.X_UnlockAll();
			Wait(vGuardLock);
		}
		vLock.X_RelockAll(uLockCount);
	}
};

}

#endif
