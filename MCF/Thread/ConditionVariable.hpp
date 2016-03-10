// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Atomic.hpp"
#include <cstddef>
#include <type_traits>
#include <cstdint>

namespace MCF {

class ConditionVariable : MCF_NONCOPYABLE {
private:
	Atomic<std::size_t> x_uControl;

public:
	constexpr ConditionVariable() noexcept
		: x_uControl(0)
	{
	}

public:
	bool Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64UntilFastMonoClock) noexcept;
	void Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept;

	std::size_t Signal(std::size_t uMaxToWakeUp = 1) noexcept;
	std::size_t Broadcast() noexcept;
};

static_assert(std::is_trivially_destructible<ConditionVariable>::value, "Hey!");

}

#endif
