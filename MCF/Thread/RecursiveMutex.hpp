// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_USER_RECURSIVE_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include <cstddef>

namespace MCF {

struct RecursiveMutexResults {
	enum Result {
		kResTryFailed		= 0,
		kResStateChanged	= 1,
		kResRecursive		= 2,
	};
};

class RecursiveMutex : NONCOPYABLE, public RecursiveMutexResults {
public:
	using UniqueLock = UniqueLockTemplate<RecursiveMutex>;

private:
	Mutex x_vMutex;
	std::size_t x_uRecursionCount;

public:
	explicit RecursiveMutex(std::size_t uSpinCount = 0x40);

public:
	std::size_t GetSpinCount() const noexcept {
		return x_vMutex.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_vMutex.SetSpinCount(uSpinCount);
	}

	bool IsLockedByCurrentThread() const noexcept {
		return x_vMutex.IsLockedByCurrentThread();
	}

	Result Try() noexcept;
	Result Lock() noexcept;
	Result Unlock() noexcept;

	std::size_t UncheckedGetRecursionCount() const noexcept {
		ASSERT(IsLockedByCurrentThread());
		return x_uRecursionCount;
	}
	std::size_t GetRecursionCount() const noexcept {
		if(!IsLockedByCurrentThread()){
			return 0;
		}
		return UncheckedGetRecursionCount();
	}

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
