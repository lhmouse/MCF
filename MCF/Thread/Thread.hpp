// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Function/Function.hpp"
#include "../Core/UniqueWin32Handle.hpp"
#include "Atomic.hpp"
#include <exception>
#include <cstddef>
#include <cstdint>

namespace MCF {

class Thread : NONCOPYABLE, public IntrusiveBase<Thread> {
public:
	static std::size_t GetCurrentId() noexcept;

	static IntrusivePtr<Thread> Create(Function<void ()> fnProc, bool bSuspended = false);

private:
	const Function<void ()> $fnProc;

	UniqueWin32Handle $hThread;
	Atomic<unsigned long> $ulThreadId;
	std::exception_ptr $pException;

private:
	Thread(Function<void ()> fnProc, bool bSuspended);

public:
	~Thread(); // 如果有被捕获的异常，调用 std::terminate()。

public:
	bool Wait(std::uint64_t u64MilliSeconds) const noexcept;
	void Wait() const noexcept;

	std::exception_ptr JoinNoThrow() const noexcept;
	void Join(); // 如果线程中有被捕获的异常，抛出异常。

	bool IsAlive() const noexcept;
	std::size_t GetId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
