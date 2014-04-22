// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_HPP_
#define MCF_THREAD_HPP_

#include "Utilities.hpp"
#include <functional>
#include <memory>
#include <cstddef>

namespace MCF {

class Thread : NO_COPY, ABSTRACT {
public:
	static std::shared_ptr<Thread> Create(std::function<void ()> fnProc, bool bSuspended = false);

public:
	bool WaitTimeout(unsigned long ulMilliSeconds) const noexcept;
	void Wait() const noexcept;
	void Join() const; // 如果线程中有被捕获的异常，抛出异常。

	bool IsAlive() const noexcept;
	unsigned long GetThreadId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
