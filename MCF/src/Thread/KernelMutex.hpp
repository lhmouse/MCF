// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_MUTEX_HPP_
#define MCF_THREAD_KERNEL_MUTEX_HPP_

#include "../Core/StringView.hpp"
#include "../Core/_KernelObjectBase.hpp"
#include "UniqueLock.hpp"
#include <cstdint>

namespace MCF {

// 由一个线程锁定的互斥锁可以由另一个线程解锁。

class KernelMutex : public Impl_KernelObjectBase::KernelObjectBase {
private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hEvent;

public:
	KernelMutex()
		: KernelMutex(nullptr, 0)
	{
	}
	KernelMutex(const WideStringView &wsvName, std::uint32_t u32Flags);

	KernelMutex(const KernelMutex &) = delete;
	KernelMutex &operator=(const KernelMutex &) = delete;

public:
	Handle GetHandle() const noexcept {
		return x_hEvent.Get();
	}

	bool Try(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock<KernelMutex> TryGetLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<KernelMutex>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<KernelMutex> GetLock() noexcept {
		return UniqueLock<KernelMutex>(*this);
	}
};

}

#endif
