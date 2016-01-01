// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Function/Function.hpp"
#include "../Core/UniqueHandle.hpp"
#include "Atomic.hpp"
#include <exception>
#include <cstddef>
#include <cstdint>

namespace MCF {

class Thread : NONCOPYABLE, public IntrusiveBase<Thread> {
public:
	using Handle = void *;

private:
	struct X_ThreadCloser {
		constexpr Handle operator()() const noexcept {
			return nullptr;
		}
		void operator()(Handle hThread) const noexcept;
	};

public:
	static IntrusivePtr<Thread> Create(Function<void ()> fnProc, bool bSuspended = false);

	static std::uintptr_t GetCurrentId() noexcept;

	static void Sleep(std::uint64_t u64UntilFastMonoClock) noexcept;
	static bool AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept;
	static void AlertableSleep() noexcept;
	static void YieldExecution() noexcept;

private:
	const Function<void ()> x_fnProc;

	UniqueHandle<X_ThreadCloser> x_hThread;
	Atomic<std::uintptr_t> x_uThreadId;
	std::exception_ptr x_pException;

private:
	Thread(Function<void ()> fnProc, bool bSuspended);

public:
	~Thread(); // 如果有被捕获的异常，调用 std::terminate()。

public:
	Handle GetHandle() const noexcept {
		return x_hThread.Get();
	}

	bool Wait(std::uint64_t u64UntilFastMonoClock) const noexcept;
	void Wait() const noexcept;

	const std::exception_ptr &GetException() const noexcept {
		return x_pException;
	}
	void ClearException() noexcept {
		x_pException = std::exception_ptr();
	}

	bool IsAlive() const noexcept;
	std::uintptr_t GetId() const noexcept;

	void Suspend() noexcept;
	void Resume() noexcept;
};

}

#endif
