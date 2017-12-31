// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_SEMAPHORE_HPP_
#define MCF_THREAD_KERNEL_SEMAPHORE_HPP_

#include "../Core/StringView.hpp"
#include "../Core/_KernelObjectBase.hpp"
#include <cstdint>

namespace MCF {

class KernelSemaphore : public Impl_KernelObjectBase::KernelObjectBase {
private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hSemaphore;

public:
	explicit KernelSemaphore(std::size_t uInitCount)
		: KernelSemaphore(uInitCount, nullptr, 0)
	{ }
	KernelSemaphore(std::size_t uInitCount, const WideStringView &wsvName, std::uint32_t u32Flags);

	KernelSemaphore(const KernelSemaphore &) = delete;
	KernelSemaphore &operator=(const KernelSemaphore &) = delete;

public:
	Handle GetHandle() const noexcept {
		return x_hSemaphore.Get();
	}

	bool Wait(std::uint64_t u64UntilFastMonoClock) noexcept;
	void Wait() noexcept;
	std::size_t Post(std::size_t uPostCount = 1) noexcept;
};

}

#endif
