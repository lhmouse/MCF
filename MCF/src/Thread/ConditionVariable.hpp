// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Core/Assert.hpp"
#include "../Core/Atomic.hpp"
#include "UniqueLock.hpp"
#include <MCFCRT/env/condition_variable.h>
#include <cstddef>
#include <type_traits>
#include <cstdint>

namespace MCF {

class ConditionVariable {
public:
	enum : std::size_t {
		kSuggestedSpinCount = _MCFCRT_CONDITION_VARIABLE_SUGGESTED_SPIN_COUNT,
	};

private:
	template<typename LockT>
	static std::intptr_t X_UnlockCallback(std::intptr_t nContext) noexcept {
		const auto pLock = reinterpret_cast<LockT *>(nContext);
		const auto pMutex = pLock->Release();
		MCF_ASSERT(pMutex);
		pMutex->Unlock();
		return reinterpret_cast<std::intptr_t>(pMutex);
	}
	template<typename LockT>
	static void X_RelockCallback(std::intptr_t nContext, std::intptr_t nUnlocked) noexcept {
		const auto pLock = reinterpret_cast<LockT *>(nContext);
		const auto pMutex = reinterpret_cast<std::decay_t<decltype(pLock->Release())>>(nUnlocked);
		MCF_ASSERT(pMutex);
		pLock->Reset(*pMutex);
	}

private:
	::_MCFCRT_ConditionVariable x_vCond;
	Atomic<std::size_t> x_uSpinCount;

public:
	explicit constexpr ConditionVariable(std::size_t uSpinCount = kSuggestedSpinCount) noexcept
		: x_vCond{ 0 }, x_uSpinCount(uSpinCount)
	{
	}

	ConditionVariable(const ConditionVariable &) = delete;
	ConditionVariable &operator=(const ConditionVariable &) = delete;

public:
	std::size_t GetSpinCount() const noexcept {
		return x_uSpinCount.Load(kAtomicRelaxed);
	}
	void SetSpinCount(std::size_t uSpinCount) noexcept {
		x_uSpinCount.Store(uSpinCount, kAtomicRelaxed);
	}

	template<typename LockT>
	bool Wait(LockT &vLock, std::uint64_t u64UntilFastMonoClock){
		return ::_MCFCRT_WaitForConditionVariable(&x_vCond, &X_UnlockCallback<LockT>, &X_RelockCallback<LockT>, reinterpret_cast<std::intptr_t>(AddressOf(vLock)), GetSpinCount(), u64UntilFastMonoClock);
	}
	template<typename LockT>
	bool WaitOrAbandon(LockT &vLock, std::uint64_t u64UntilFastMonoClock){
		return ::_MCFCRT_WaitForConditionVariableOrAbandon(&x_vCond, &X_UnlockCallback<LockT>, &X_RelockCallback<LockT>, reinterpret_cast<std::intptr_t>(AddressOf(vLock)), GetSpinCount(), u64UntilFastMonoClock);
	}
	template<typename LockT>
	void Wait(LockT &vLock){
		::_MCFCRT_WaitForConditionVariableForever(&x_vCond, &X_UnlockCallback<LockT>, &X_RelockCallback<LockT>, reinterpret_cast<std::intptr_t>(AddressOf(vLock)), GetSpinCount());
	}

	std::size_t Signal(std::size_t uMaxCountToWakeUp = 1) noexcept {
		return ::_MCFCRT_SignalConditionVariable(&x_vCond, uMaxCountToWakeUp);
	}
	std::size_t Broadcast() noexcept {
		return ::_MCFCRT_BroadcastConditionVariable(&x_vCond);
	}
};

static_assert(std::is_trivially_destructible<ConditionVariable>::value, "Hey!");

}

#endif
