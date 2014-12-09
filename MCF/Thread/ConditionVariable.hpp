// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"

namespace MCF {

class UserMutex;

class ConditionVariable : NONCOPYABLE {
private:
	UserMutex &xm_vMutex;

	unsigned long xm_ulWaiting;
	Semaphore xm_vSemaphore;

public:
	explicit ConditionVariable(UserMutex &vMutex);

public:
	bool Wait(unsigned long long ullMilliSeconds) noexcept;
	void Wait() noexcept;
	void Signal(unsigned long ulMaxCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
