// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_MUTEX_HPP_
#define MCF_THREAD_USER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "UniqueLockTemplate.hpp"
#include "Semaphore.hpp"
#include "_SpinLock.hpp"
#include <cstddef>

namespace MCF {

class Mutex : NONCOPYABLE {
public:
	using UniqueLock = UniqueLockTemplate<Mutex>;

private:
	Semaphore xm_vSemaphore;
	volatile std::size_t xm_uSpinCount;

	SpinLock xm_splQueueSize;
	volatile std::size_t xm_uLockingThreadId;

public:
	explicit Mutex(std::size_t uSpinCount = 0x400);

private:
	bool xTryWithHint(unsigned long ulThreadId) noexcept;

public:
	std::size_t GetSpinCount() const noexcept {
		return __atomic_load_n(&xm_uSpinCount, __ATOMIC_RELAXED);
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept;

	bool IsLockedByCurrentThread() const noexcept;

	bool Try() noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock TryLock() noexcept;
	UniqueLock GetLock() noexcept;
};

}

#endif
