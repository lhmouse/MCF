// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_MUTEX_HPP_
#define MCF_THREAD_USER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "Atomic.hpp"
#include "UniqueLockTemplate.hpp"
#include "Semaphore.hpp"
#include <cstddef>

namespace MCF {

class Mutex : NONCOPYABLE {
private:
	struct xQueueNode;

public:
	using UniqueLock = UniqueLockTemplate<Mutex>;

private:
	Semaphore x_vSemaphore;
	volatile std::size_t x_uSpinCount;

	volatile std::size_t x_uLockingThreadId;
	xQueueNode *volatile x_pQueueHead;

public:
	explicit Mutex(std::size_t uSpinCount = 0x40);

private:
	bool xIsQueueEmpty() const noexcept;
	xQueueNode *xLockQueue() noexcept;
	void xUnlockQueue(xQueueNode *pQueueHead) noexcept;

public:
	std::size_t GetSpinCount() const noexcept {
		return AtomicLoad(x_uSpinCount, MemoryModel::kRelaxed);
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept;

	bool IsLockedByCurrentThread() const noexcept;

	bool Try() noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock TryLock() noexcept {
		UniqueLock vLock(*this, false);
		vLock.Try();
		return vLock;
	}
	UniqueLock GetLock() noexcept {
		return UniqueLock(*this);
	}
};

}

#endif
