// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "UniqueLockTemplate.hpp"
#include "../Utilities/Noncopyable.hpp"
#include "Semaphore.hpp"

namespace MCF {

class ConditionVariable : NONCOPYABLE {
private:
	Semaphore xm_vSemaphore;
	volatile unsigned long xm_ulWaiting;

public:
	ConditionVariable();

public:
	bool Wait(UniqueLockTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept;
	void Wait(UniqueLockTemplateBase &vLock) noexcept;
	void Signal(unsigned long ulMaxCount = 1) noexcept;
	void SignalAll() noexcept;
};

}

#endif
