// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Function/Function.hpp"
#include "_UniqueWin32Handle.hpp"
#include <exception>
#include <cstddef>

namespace MCF {

class Thread : NONCOPYABLE, public IntrusiveBase<Thread> {
public:
	static std::size_t GetCurrentId() noexcept;

	static IntrusivePtr<Thread> Create(Function<void ()> fnProc, bool bSuspended = false);

private:
	const Function<void ()> x_fnProc;

	UniqueWin32Handle x_hThread;
	volatile unsigned long x_ulThreadId;
	std::exception_ptr x_pException;

private:
	Thread(Function<void ()> fnProc, bool bSuspended);

public:
	bool Wait(unsigned long long ullMilliSeconds) const noexcept;
	void Wait() const noexcept;

	std::exception_ptr JoinNoThrow() const noexcept;
	void Join() const; // 如果线程中有被捕获的异常，抛出异常。

	bool IsAlive() const noexcept;
	std::size_t GetId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
