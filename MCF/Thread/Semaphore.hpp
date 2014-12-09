// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_SEMAPHORE_HPP_
#define MCF_THREAD_SEMAPHORE_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/String.hpp"
#include "_Win32Handle.hpp"

namespace MCF {

class Semaphore : NONCOPYABLE {
private:
	const UniqueWin32Handle xm_hSemaphore;

public:
	explicit Semaphore(unsigned long ulInitCount, const wchar_t *pwszName = nullptr);
	Semaphore(unsigned long ulInitCount, const WideString &wsName);

public:
	unsigned long Wait(unsigned long long ullMilliSeconds) noexcept;
	void Wait() noexcept;
	unsigned long Post(unsigned long ulPostCount = 1) noexcept;

	unsigned long BatchWait(unsigned long long ullMilliSeconds, unsigned long ulWaitCount) noexcept;
	void BatchWait(unsigned long ulWaitCount) noexcept;
};

}

#endif
