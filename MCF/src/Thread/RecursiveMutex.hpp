// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_RECURSIVE_MUTEX_HPP_

#include "../Core/Noncopyable.hpp"
#include "../Core/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"

namespace MCF {

class RecursiveMutex : MCF_NONCOPYABLE {
public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<RecursiveMutex>;

private:
	Mutex x_vMutex;
	Atomic<std::uintptr_t> x_uLockingThreadId;
	std::size_t x_uRecursionCount;

public:
	explicit constexpr RecursiveMutex(std::size_t uSpinCount = Mutex::kSuggestedSpinCount) noexcept
		: x_vMutex(uSpinCount)
		, x_uLockingThreadId(0), x_uRecursionCount(0)
	{
	}

public:
	std::size_t GetSpinCount() const noexcept {
		return x_vMutex.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_vMutex.SetSpinCount(uSpinCount);
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

	UniqueLock TryGetLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		UniqueLock vLock(*this, false);
		vLock.Try(u64UntilFastMonoClock);
		return vLock;
	}
	UniqueLock GetLock() noexcept {
		return UniqueLock(*this);
	}
};

static_assert(std::is_trivially_destructible<RecursiveMutex>::value, "Hey!");

namespace Impl_UniqueLockTemplate {
	template<>
	inline bool RecursiveMutex::UniqueLock::X_DoTry(std::uint64_t u64UntilFastMonoClock) const noexcept {
		return x_pOwner->Try(u64UntilFastMonoClock);
	}
	template<>
	inline void RecursiveMutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	inline void RecursiveMutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}
}

#endif
