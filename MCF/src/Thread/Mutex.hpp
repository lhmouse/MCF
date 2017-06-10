// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MUTEX_HPP_
#define MCF_THREAD_MUTEX_HPP_

#include "../Core/Assert.hpp"
#include "../Core/Atomic.hpp"
#include "UniqueLock.hpp"
#include <MCFCRT/env/mutex.h>
#include <type_traits>
#include <cstddef>

namespace MCF {

// 由一个线程锁定的互斥锁可以由另一个线程解锁。

class Mutex {
public:
	enum : std::size_t {
		kSuggestedSpinCount = _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT,
	};

private:
	::_MCFCRT_Mutex x_vMutex;
	Atomic<std::size_t> x_uSpinCount;

public:
	explicit constexpr Mutex(std::size_t uSpinCount = kSuggestedSpinCount) noexcept
		: x_vMutex{ 0 }, x_uSpinCount(uSpinCount)
	{ }

	Mutex(const Mutex &) = delete;
	Mutex &operator=(const Mutex &) = delete;

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

	UniqueLock<Mutex> TryGetLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<Mutex>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<Mutex> GetLock() noexcept {
		return UniqueLock<Mutex>(*this);
	}
};

static_assert(std::is_trivially_destructible<Mutex>::value, "Hey!");

}

#endif
