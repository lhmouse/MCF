// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Core/IntrusivePtr.hpp"
#include "Win32Handle.hpp"
#include <functional>
#include <exception>
#include <cstddef>

namespace MCF {

class Thread : public IntrusiveBase<Thread>, Noncopyable {
public:
	static unsigned long GetCurrentId() noexcept;

	static IntrusivePtr<Thread> Create(std::function<void ()> fnProc, bool bSuspended = false);

private:
	const std::function<void ()> xm_fnProc;

	UniqueWin32Handle xm_hThread;
	volatile unsigned long xm_ulThreadId;
	std::exception_ptr xm_pException;

private:
	explicit Thread(std::function<void ()> fnProc);

public:
	bool Wait(unsigned long long ullMilliSeconds) const noexcept;
	void Wait() const noexcept;

	std::exception_ptr JoinNoThrow() const noexcept;
	void Join() const; // 如果线程中有被捕获的异常，抛出异常。

	bool IsAlive() const noexcept;
	unsigned long GetId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
