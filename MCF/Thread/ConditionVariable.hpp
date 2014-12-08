// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "LockRaiiTemplate.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"

namespace MCF {

class ConditionVariable : Noncopyable {
private:
	Semaphore xm_vSemaphore;
	volatile unsigned long xm_ulWaiting;

public:
	ConditionVariable();

public:
	bool Wait(LockRaiiTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept;
	void Wait(LockRaiiTemplateBase &vLock) noexcept;
	void Signal(unsigned long ulMaxCount = 1) noexcept;
	void SignalAll() noexcept;
};

}

#endif
