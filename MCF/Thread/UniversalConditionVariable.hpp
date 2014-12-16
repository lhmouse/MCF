// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_UNIVERSAL_CONDITION_VARIABLE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "UniqueLockTemplate.hpp"
#include "Mutex.hpp"
#include "ConditionVariable.hpp"
#include <cstddef>

namespace MCF {

class UniversalConditionVariable : NONCOPYABLE {
private:
	Mutex xm_vMutex;
	ConditionVariable xm_vDelegate;

public:
	explicit UniversalConditionVariable(std::size_t uSpinCount = 0x400);

public:
	bool Wait(UniqueLockTemplateBase &vLock, unsigned long long ullMilliSeconds) noexcept;
	void Wait(UniqueLockTemplateBase &vLock) noexcept;
	void Signal(std::size_t uMaxCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
