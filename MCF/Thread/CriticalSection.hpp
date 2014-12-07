// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CRITICAL_SECTION_HPP_
#define MCF_THREAD_CRITICAL_SECTION_HPP_

#include "LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "Semaphore.hpp"

namespace MCF {

struct CriticalSectionResults {
	enum Result {
		R_TRY_FAILED	= 0,
		R_STATE_CHANGED	= 1,
		R_RECURSIVE		= 2,
	};
};

class CriticalSection : NO_COPY, public CriticalSectionResults {
public:
	using Lock = LockRaiiTemplate<CriticalSection>;

private:
	Semaphore xm_vSemaphore;
	volatile unsigned long xm_ulSpinCount;

	volatile unsigned long xm_ulQueueSize;
	volatile unsigned long xm_ulLockingThreadId;
	unsigned long xm_ulRecursionCount;

public:
	explicit CriticalSection(unsigned long ulSpinCount = 0x400);

private:
	bool xNonRecursiveTry(unsigned long ulThreadId) noexcept;
	void xNonRecursiveAcquire(unsigned long ulThreadId) noexcept;
	void xNonRecursiveRelease() noexcept;

public:
	unsigned long GetSpinCount() const noexcept {
		return __atomic_load_n(&xm_ulSpinCount, __ATOMIC_RELAXED);
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		__atomic_store_n(&xm_ulSpinCount, ulSpinCount, __ATOMIC_RELAXED);
	}

	Result Try() noexcept;
	Result Acquire() noexcept;
	Result Release() noexcept;

	bool IsLockedByCurrentThread() const noexcept;
	// 要求 IsLockedByCurrentThread() != false。
	unsigned long UncheckedGetRecursionCount() const noexcept;
	unsigned long GetRecursionCount() const noexcept;

	Lock TryLock() noexcept;
	Lock GetLock() noexcept;
};

}

#endif
