// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_RECURSIVE_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include <cstddef>

namespace MCF {

class RecursiveMutex : NONCOPYABLE {
public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<RecursiveMutex>;

private:
	Mutex x_vMutex;
	Atomic<std::size_t> x_uLockingThreadId;
	std::size_t x_uRecursionCount;

public:
	explicit constexpr RecursiveMutex(std::size_t uSpinCount = Mutex::kDefaultSpinCount) noexcept
		: x_vMutex(uSpinCount)
		, x_uLockingThreadId(0), x_uRecursionCount(0)
	{
	}
	~RecursiveMutex(){
		ASSERT(x_uLockingThreadId.Load(kAtomicConsume) == 0);
	}

public:
	std::size_t GetSpinCount() const noexcept {
		return x_vMutex.GetSpinCount();
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_vMutex.SetSpinCount(uSpinCount);
	}

	bool IsLockedByCurrentThread() const noexcept;

	bool Try(std::uint64_t u64UntilUtcTime) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	std::size_t GetRecursionCount() const noexcept {
		if(!IsLockedByCurrentThread()){
			return 0;
		}
		return UncheckedGetRecursionCount();
	}
	std::size_t UncheckedGetRecursionCount() const noexcept {
		ASSERT(IsLockedByCurrentThread());

		return x_uRecursionCount;
	}

	UniqueLock TryGetLock(std::uint64_t u64UntilUtcTime = 0) noexcept {
		UniqueLock vLock(*this, false);
		vLock.Try(u64UntilUtcTime);
		return vLock;
	}
	UniqueLock GetLock() noexcept {
		return UniqueLock(*this);
	}
};


namespace Impl_UniqueLockTemplate {
	template<>
	inline bool RecursiveMutex::UniqueLock::X_DoTry(std::uint64_t u64UntilUtcTime) const noexcept {
		return x_pOwner->Try(u64UntilUtcTime);
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
