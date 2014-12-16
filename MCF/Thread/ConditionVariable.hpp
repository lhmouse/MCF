// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"
#include <cstddef>

namespace MCF {

class Mutex;

class ConditionVariable : NONCOPYABLE {
private:
	Mutex &xm_vMutex;

	std::size_t xm_uWaiting;
	Semaphore xm_vSemaphore;

public:
	explicit ConditionVariable(Mutex &vMutex);

public:
	bool Wait(unsigned long long ullMilliSeconds) noexcept;
	void Wait() noexcept;
	void Signal(std::size_t uMaxCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
