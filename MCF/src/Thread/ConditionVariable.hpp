// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Config.hpp"
#include <MCFCRT/env/condition_variable.h>
#include "../Core/Noncopyable.hpp"
#include "../Core/Assert.hpp"
#include "UniqueLockBase.hpp"
#include <cstddef>
#include <type_traits>
#include <cstdint>

namespace MCF {

class ConditionVariable : MCF_NONCOPYABLE {
private:
	static std::intptr_t X_UnlockCallback(std::intptr_t nContext) noexcept;
	static void X_RelockCallback(std::intptr_t nContext, std::intptr_t nUnlocked) noexcept;

private:
	::_MCFCRT_ConditionVariable x_vConditionVariable;

public:
	constexpr ConditionVariable() noexcept
		: x_vConditionVariable{ 0 }
	{
	}

public:
	bool Wait(UniqueLockBase &vLock, std::uint64_t u64UntilFastMonoClock) noexcept {
		return ::_MCFCRT_WaitForConditionVariable(&x_vConditionVariable, &X_UnlockCallback, &X_RelockCallback, reinterpret_cast<std::intptr_t>(&vLock), u64UntilFastMonoClock);
	}
	void Wait(UniqueLockBase &vLock) noexcept {
		::_MCFCRT_WaitForConditionVariableForever(&x_vConditionVariable, &X_UnlockCallback, &X_RelockCallback, reinterpret_cast<std::intptr_t>(&vLock));
	}

	std::size_t Signal(std::size_t uMaxCountToWakeUp = 1) noexcept {
		return ::_MCFCRT_SignalConditionVariable(&x_vConditionVariable, uMaxCountToWakeUp);
	}
	std::size_t Broadcast() noexcept {
		return ::_MCFCRT_BroadcastConditionVariable(&x_vConditionVariable);
	}
};

static_assert(std::is_trivially_destructible<ConditionVariable>::value, "Hey!");

}

#endif
