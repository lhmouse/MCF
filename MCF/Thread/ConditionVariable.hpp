// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"
#include "Atomic.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class Mutex;

class ConditionVariable : NONCOPYABLE {
private:
	Mutex &x_vMutex;

	Atomic<std::size_t> x_uWaiting;
	Semaphore x_vSemaphore;

public:
	explicit ConditionVariable(Mutex &vMutex);

public:
	bool Wait(std::uint64_t u64MilliSeconds) noexcept;
	void Wait() noexcept;
	void Signal(std::size_t uMaxCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
