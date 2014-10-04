// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CONDITION_VARIABLE_HPP_
#define MCF_THREAD_CONDITION_VARIABLE_HPP_

#include "_LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include <memory>

namespace MCF {

class ConditionVariable : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<ConditionVariable> Create();

public:
	bool WaitTimeout(LockRaiiTemplateBase &vLockRaiiTemplate, unsigned long long ullMilliSeconds) noexcept;
	void Wait(LockRaiiTemplateBase &vLockRaiiTemplate) noexcept;
	void Signal(std::size_t uCount = 1) noexcept;
	void Broadcast() noexcept;
};

}

#endif
