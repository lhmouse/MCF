// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_EVENT_HPP_
#define MCF_THREAD_KERNEL_EVENT_HPP_

#include "../Core/StringView.hpp"
#include "../Core/_KernelObjectBase.hpp"
#include <cstdint>

namespace MCF {

class KernelEvent : public Impl_KernelObjectBase::KernelObjectBase {
public:
	using Handle = Impl_UniqueNtHandle::Handle;

private:
	static Impl_UniqueNtHandle::UniqueNtHandle X_CreateEventHandle(bool bInitSet, const WideStringView &wsvName, std::uint32_t u32Flags);

private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hEvent;

public:
	explicit KernelEvent(bool bInitSet)
		: x_hEvent(X_CreateEventHandle(bInitSet, nullptr, kSessionLocal))
	{
	}
	KernelEvent(bool bInitSet, const WideStringView &wsvName, std::uint32_t u32Flags)
		: x_hEvent(X_CreateEventHandle(bInitSet, wsvName, u32Flags))
	{
	}

public:
	Handle GetHandle() const noexcept {
		return x_hEvent.Get();
	}

	bool Wait(std::uint64_t u64UntilFastMonoClock) const noexcept;
	void Wait() const noexcept;
	bool IsSet() const noexcept;
	bool Set() noexcept;
	bool Reset() noexcept;
};

}

#endif
