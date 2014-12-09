// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "UniqueLockTemplate.hpp"
#include "ConditionVariable.hpp"
#include "UserMutex.hpp"

namespace MCF {

class UniversalConditionVariable : NONCOPYABLE {
private:
	UserMutex xm_vMutex;
	ConditionVariable xm_vDelegate;

public:
	explicit UniversalConditionVariable(unsigned long ulSpinCount = 0x400);

public:
	bool Wait(UniqueLockTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept;
	void Wait(UniqueLockTemplateBase &vLock) noexcept;
	void Signal(unsigned long ulMaxCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
