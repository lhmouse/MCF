// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_RECURSIVE_MUTEX_HPP_

#include "Mutex.hpp"

namespace MCF {

class RecursiveMutex {
private:
	Mutex x_mtxPlain;
	Atomic<std::uintptr_t> x_uLockingThreadId;
	std::size_t x_uRecursionCount;

public:
	explicit constexpr RecursiveMutex(std::size_t uSpinCount = Mutex::kSuggestedSpinCount) noexcept
		: x_mtxPlain(uSpinCount), x_uLockingThreadId(0), x_uRecursionCount(0)
	{
	}

	RecursiveMutex(const RecursiveMutex &) = delete;
	RecursiveMutex &operator=(const RecursiveMutex &) = delete;

public:
	std::size_t GetSpinCount() const noexcept {
		return x_mtxPlain.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_mtxPlain.SetSpinCount(uSpinCount);
	}

	std::uintptr_t GetLockingThreadId() const noexcept {
		return x_uLockingThreadId.Load(kAtomicRelaxed);
	}
	bool IsLockedByCurrentThread() const noexcept;

	bool Try(std::uint64_t u64UntilFastMonoClock) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	std::size_t GetRecursionCount() const noexcept {
		if(!IsLockedByCurrentThread()){
			return 0;
		}
		return UncheckedGetRecursionCount();
	}
	std::size_t UncheckedGetRecursionCount() const noexcept {
		MCF_DEBUG_CHECK(IsLockedByCurrentThread());

		return x_uRecursionCount;
	}

	UniqueLock<RecursiveMutex> TryGetLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<RecursiveMutex>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<RecursiveMutex> GetLock() noexcept {
		return UniqueLock<RecursiveMutex>(*this);
	}
};

static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Hey!");

}

#endif
