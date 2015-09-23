// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_EVENT_HPP_
#define MCF_THREAD_KERNEL_EVENT_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/StringObserver.hpp"
#include "_UniqueNtHandle.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class KernelEvent : NONCOPYABLE {
private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hEvent;

public:
	explicit KernelEvent(bool bInitSet, WideStringObserver wsoName = nullptr);

public:
	bool Wait(std::uint64_t u64MilliSeconds) const noexcept;
	void Wait() const noexcept;
	bool IsSet() const noexcept;
	bool Set() noexcept;
	bool Reset() noexcept;
};

}

#endif
