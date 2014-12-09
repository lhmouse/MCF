// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_

#include "UniqueLockTemplate.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "UserMutex.hpp"

namespace MCF {

enum UserRecursiveMutexResult {
	R_TRY_FAILED	= 0,
	R_STATE_CHANGED	= 1,
	R_RECURSIVE		= 2,
};

class UserRecursiveMutex : NONCOPYABLE {
public:
	using Result = UserRecursiveMutexResult;
	using UniqueLock = UniqueLockTemplate<UserRecursiveMutex>;

private:
	UserMutex xm_vMutex;
	unsigned long xm_ulRecursionCount;

public:
	explicit UserRecursiveMutex(unsigned long ulSpinCount = 0x400);

public:
	unsigned long GetSpinCount() const noexcept {
		return xm_vMutex.GetSpinCount();
	}
	void SetSpinCount(unsigned long ulSpinCount) noexcept {
		xm_vMutex.SetSpinCount(ulSpinCount);
	}

	bool IsLockedByCurrentThread() const noexcept {
		return xm_vMutex.IsLockedByCurrentThread();
	}

	Result Try() noexcept;
	Result Lock() noexcept;
	Result Unlock() noexcept;

	// 要求 IsLockedByCurrentThread() != false。
	unsigned long UncheckedGetRecursionCount() const noexcept;
	unsigned long GetRecursionCount() const noexcept;

	UniqueLock TryLock() noexcept;
	UniqueLock GetLock() noexcept;
};

}

#endif
