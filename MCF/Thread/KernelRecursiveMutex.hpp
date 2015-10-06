// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_RECURSIVE_MUTEX_HPP_
#define MCF_THREAD_KERNEL_RECURSIVE_MUTEX_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/StringView.hpp"
#include "_UniqueLockTemplate.hpp"
#include "_UniqueNtHandle.hpp"
#include <cstdint>

namespace MCF {

class KernelRecursiveMutex : NONCOPYABLE {
public:
	using UniqueLock = Impl_UniqueLockTemplate::UniqueLockTemplate<KernelRecursiveMutex>;

private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hMutex;

public:
	explicit KernelRecursiveMutex(const WideStringView &wsvName = nullptr);

public:
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

}

#endif
