// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class ConditionVariable : NONCOPYABLE {
private:
	Mutex x_mtxGuard;
	Atomic<std::size_t> x_uWaitingThreads;

public:
	constexpr ConditionVariable() noexcept
		: x_mtxGuard(), x_uWaitingThreads(0)
	{
	}

public:
	bool Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept;
	void Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept;

	void Signal() noexcept;
	void Broadcast() noexcept;
};

}

#endif
