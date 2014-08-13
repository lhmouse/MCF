// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_HPP_
#define MCF_THREAD_HPP_

#include "../Utilities/Utilities.hpp"
#include <functional>
#include <memory>
#include <exception>
#include <cstddef>

namespace MCF {

class Thread : NO_COPY, ABSTRACT {
public:
	static unsigned long GetCurrentId() noexcept;

	static std::shared_ptr<Thread> Create(std::function<void ()> fnProc, bool bSuspended = false);

public:
	bool WaitTimeout(unsigned long long ullMilliSeconds) const noexcept;
	void Wait() const noexcept;

	std::exception_ptr JoinNoThrow() const;
	void Join() const; // 如果线程中有被捕获的异常，抛出异常。

	bool IsAlive() const noexcept;
	unsigned long GetId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
