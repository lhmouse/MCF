// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Atomic.hpp"
#include <cstddef>

namespace MCF {

// 由一个线程锁定的互斥锁可以由另一个线程解锁。

class ConditionVariable;

class Mutex : NONCOPYABLE {
	friend ConditionVariable;

public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<Mutex>;

private:
	Atomic<std::size_t> x_uSpinCount;
	std::uintptr_t x_uImpl;

public:
	explicit constexpr Mutex(std::size_t uSpinCount = 0x400) noexcept
		: x_uSpinCount(uSpinCount), x_uImpl(0)
	{
	}

public:
	std::size_t GetSpinCount() const noexcept {
		return x_uSpinCount.Load(kAtomicRelaxed);
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
	}

	bool Try(std::uint64_t u64MilliSeconds = 0) noexcept;
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

namespace Impl_UniqueLockTemplate {
	template<>
	inline bool Mutex::UniqueLock::X_DoTry(std::uint64_t u64MilliSeconds) const noexcept {
		return x_pOwner->Try(u64MilliSeconds);
	}
	template<>
	inline void Mutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	inline void Mutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}

}

#endif
