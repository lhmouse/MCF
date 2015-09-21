// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "_UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include "Atomic.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class ConditionVariable : NONCOPYABLE {
private:
	Mutex x_mtxGuard;
	std::intptr_t x_aImpl[1];

public:
	ConditionVariable() noexcept;

private:
	bool X_SleepOnMutex(Mutex &vMutex, std::uint64_t u64MilliSeconds) noexcept;
	bool X_SleepOnMutex(Mutex &vMutex) noexcept;

public:
	bool Wait(Mutex::UniqueLock &vLock, std::uint64_t u64MilliSeconds) noexcept;
	void Wait(Mutex::UniqueLock &vLock) noexcept;
	bool Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock, std::uint64_t u64MilliSeconds) noexcept;
	void Wait(Impl_UniqueLockTemplate::UniqueLockTemplateBase &vLock) noexcept;
	void Signal() noexcept;
	void Broadcast() noexcept;
};

}

#endif
