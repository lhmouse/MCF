// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SEMAPHORE_HPP__
#define __MCF_SEMAPHORE_HPP__

#include "NoCopy.hpp"
#include <memory>

namespace MCF {

class Semaphore : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit Semaphore(std::size_t uInitCount, std::size_t uMaxCount, const wchar_t *pwszName = nullptr);
	Semaphore(Semaphore &&rhs) noexcept;
	Semaphore &operator=(Semaphore &&rhs) noexcept;
	~Semaphore();

public:
	std::size_t WaitTimeout(unsigned long ulMilliSeconds, std::size_t uWaitCount = 1) noexcept;
	void Wait(std::size_t uWaitCount = 1) noexcept;
	void Signal(std::size_t uSignalCount = 1) noexcept;
};

}

#endif
