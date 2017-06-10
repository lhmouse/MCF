// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_KERNEL_RECURSIVE_MUTEX_HPP_

#include "../Core/StringView.hpp"
#include "../Core/_KernelObjectBase.hpp"
#include "UniqueLock.hpp"
#include <cstdint>

namespace MCF {

class KernelRecursiveMutex : public Impl_KernelObjectBase::KernelObjectBase {
private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hMutex;

public:
	KernelRecursiveMutex()
		: KernelRecursiveMutex(nullptr, 0)
	{ }
	KernelRecursiveMutex(const WideStringView &wsvName, std::uint32_t u32Flags);

	KernelRecursiveMutex(const KernelRecursiveMutex &) = delete;
	KernelRecursiveMutex &operator=(const KernelRecursiveMutex &) = delete;

public:
	Handle GetHandle() const noexcept {
		return x_hMutex.Get();
	}

	bool Try(std::uint64_t u64UntilFastMonoClock = 0) noexcept;
	void Lock() noexcept;
	void Unlock() noexcept;

	UniqueLock<KernelRecursiveMutex> TryGetLock(std::uint64_t u64UntilFastMonoClock = 0) noexcept {
		return UniqueLock<KernelRecursiveMutex>(*this, u64UntilFastMonoClock);
	}
	UniqueLock<KernelRecursiveMutex> GetLock() noexcept {
		return UniqueLock<KernelRecursiveMutex>(*this);
	}
};

}

#endif
