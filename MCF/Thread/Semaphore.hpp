// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SEMAPHORE_HPP_
#define MCF_THREAD_SEMAPHORE_HPP_

#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Abstract.hpp"
#include "../Core/String.hpp"
#include <memory>

namespace MCF {

class Semaphore : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Semaphore> Create(unsigned long ulInitCount, const wchar_t *pwszName = nullptr);
	static std::unique_ptr<Semaphore> Create(unsigned long ulInitCount, const WideString &wsName);

public:
	unsigned long WaitTimeout(unsigned long long ullMilliSeconds, unsigned long ulWaitCount = 1) noexcept;
	void Wait(unsigned long ulWaitCount = 1) noexcept;
	void Signal(unsigned long ulSignalCount = 1) noexcept;
};

}

#endif
