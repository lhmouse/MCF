// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_KERNEL_SEMAPHORE_HPP_
#define MCF_THREAD_KERNEL_SEMAPHORE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/StringView.hpp"
#include "_UniqueNtHandle.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class KernelSemaphore : NONCOPYABLE {
private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hSemaphore;

public:
	explicit KernelSemaphore(std::size_t uInitCount, const WideStringView &wsoName = nullptr);

public:
	bool Wait(std::uint64_t u64MilliSeconds) noexcept;
	void Wait() noexcept;
	std::size_t Post(std::size_t uPostCount = 1) noexcept;
};

}

#endif
