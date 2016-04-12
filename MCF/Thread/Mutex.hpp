// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "../../MCFCRT/env/mutex.h"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Atomic.hpp"
#include <type_traits>
#include <cstddef>

namespace MCF {

// 由一个线程锁定的互斥锁可以由另一个线程解锁。

class ConditionVariable;

class Mutex : MCF_NONCOPYABLE {
	friend ConditionVariable;

public:
	enum : std::size_t {
		kSuggestedSpinCount = _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT,
	};

public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<Mutex>;

private:
	::_MCFCRT_Mutex x_vMutex;
	Atomic<std::size_t> x_uSpinCount;

public:
	explicit constexpr Mutex(std::size_t uSpinCount = kSuggestedSpinCount) noexcept
		: x_vMutex{ 0 }, x_uSpinCount(uSpinCount)
	{
	}

public:
	std::size_t GetSpinCount() const noexcept {
		return x_uSpinCount.Load(kAtomicRelaxed);
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
	}

	bool Try(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return ::_MCFCRT_WaitForMutex(&x_vMutex, GetSpinCount(), u64UntilFastMonoClock);
	}
	void Lock() noexcept {
		::_MCFCRT_WaitForMutexForever(&x_vMutex, GetSpinCount());
	}
	void Unlock() noexcept {
		::_MCFCRT_SignalMutex(&x_vMutex);
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

static_assert(std::is_trivially_destructible<Mutex>::value, "Hey!");

namespace Impl_UniqueLockTemplate {
	template<>
	inline bool Mutex::UniqueLock::X_DoTry(std::uint64_t u64UntilFastMonoClock) const noexcept {
		return x_pOwner->Try(u64UntilFastMonoClock);
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
