// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_MUTEX_HPP_
#define MCF_THREAD_USER_MUTEX_HPP_

#include "UniqueLockTemplate.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"

namespace MCF {

class UserMutex : NONCOPYABLE {
public:
	using UniqueLock = UniqueLockTemplate<UserMutex>;

private:
	Semaphore xm_vSemaphore;
	volatile unsigned long xm_ulSpinCount;

	volatile unsigned long xm_ulQueueSize;
	volatile unsigned long xm_ulLockingThreadId;

public:
	explicit UserMutex(unsigned long ulSpinCount = 0x400);

private:
	bool xTryWithHint(unsigned long ulThreadId) noexcept;

public:
	unsigned long GetSpinCount() const noexcept {
		return __atomic_load_n(&xm_ulSpinCount, __ATOMIC_RELAXED);
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		__atomic_store_n(&xm_ulSpinCount, ulSpinCount, __ATOMIC_RELAXED);
	}

	bool IsLockedByCurrentThread() const noexcept;

	bool Try() noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock TryLock() noexcept;
	UniqueLock GetLock() noexcept;
};

}

#endif
