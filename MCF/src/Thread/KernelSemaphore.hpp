// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_SEMAPHORE_HPP_
#define MCF_THREAD_KERNEL_SEMAPHORE_HPP_

#include "../Config.hpp"
#include "../Core/StringView.hpp"
#include "../Core/_KernelObjectBase.hpp"
#include <cstdint>

namespace MCF {

class KernelSemaphore : public Impl_KernelObjectBase::KernelObjectBase {
public:
	using Handle = Impl_UniqueNtHandle::Handle;

private:
	static Impl_UniqueNtHandle::UniqueNtHandle X_CreateSemaphoreHandle(std::size_t uInitCount, const WideStringView &wsvName, std::uint32_t u32Flags);

private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hSemaphore;

public:
	explicit KernelSemaphore(std::size_t uInitCount)
		: x_hSemaphore(X_CreateSemaphoreHandle(uInitCount, nullptr, kSessionLocal))
	{
	}
	KernelSemaphore(std::size_t uInitCount, const WideStringView &wsvName, std::uint32_t u32Flags)
		: x_hSemaphore(X_CreateSemaphoreHandle(uInitCount, wsvName, u32Flags))
	{
	}

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
