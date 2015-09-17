// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_MUTEX_HPP_
#define MCF_THREAD_USER_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Atomic.hpp"
#include "Semaphore.hpp"
#include <cstddef>

namespace MCF {

// 由一个线程锁定的互斥锁可以由另一个线程解锁。

class Mutex : NONCOPYABLE {
private:
	struct X_QueueNode;

public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<Mutex>;

private:
	Atomic<std::size_t> x_uSpinCount;

	Atomic<std::size_t> x_uLockingThreadId; // 如果带错误检测，这个是锁定的线程 ID；否则是一个布尔值，表示是否已被锁定。
	Semaphore x_vSemaphore;
	Atomic<X_QueueNode *> x_pQueueHead;

public:
	explicit Mutex(std::size_t uSpinCount = 0x100);

private:
	bool X_IsQueueEmpty() const noexcept;
	X_QueueNode *X_LockQueue() noexcept;
	void X_UnlockQueue(X_QueueNode *pQueueHead) noexcept;

public:
	std::size_t GetSpinCount() const noexcept;
	void SetSpinCount(std::size_t uSpinCount) noexcept;

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
