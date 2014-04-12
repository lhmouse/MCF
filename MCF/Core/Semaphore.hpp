// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SEMAPHORE_HPP__
#define __MCF_SEMAPHORE_HPP__

#include "Utilities.hpp"
#include "StringObserver.hpp"
#include <memory>

namespace MCF {

class Semaphore : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<Semaphore> Create(unsigned long ulInitCount, unsigned long ulMaxCount, const WideStringObserver &wsoName = nullptr);

public:
	unsigned long WaitTimeout(unsigned long ulMilliSeconds, unsigned long ulWaitCount = 1) noexcept;
	void Wait(unsigned long ulWaitCount = 1) noexcept;
	void Signal(unsigned long ulSignalCount = 1) noexcept;
};

}

#endif
