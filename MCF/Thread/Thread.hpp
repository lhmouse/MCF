// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Core/UniqueHandle.hpp"
#include "Atomic.hpp"
#include <exception>
#include <type_traits>
#include <cstddef>

namespace MCF {

class Thread : MCF_NONCOPYABLE, public IntrusiveBase<Thread> {
private:
	template<typename ThreadProcT>
	class X_ConcreteThread;

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
	template<typename ThreadProcT>
	static IntrusivePtr<Thread> Create(ThreadProcT &&fnProc, bool bSuspended = false);

	static std::uintptr_t GetCurrentId() noexcept;

	static void Sleep(std::uint64_t u64UntilFastMonoClock) noexcept;
	static bool AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept;
	static void AlertableSleep() noexcept;
	static void YieldExecution() noexcept;

private:
	UniqueHandle<X_ThreadCloser> x_hThread;
	Atomic<std::uintptr_t> x_uThreadId;
	std::exception_ptr x_pException;

private:
	Thread() noexcept
		: x_hThread(nullptr), x_uThreadId(0), x_pException()
	{
	}

public:
	~Thread() override; // 如果有被捕获的异常，调用 std::terminate()。

private:
	void X_Initialize(bool bSuspended);

	virtual void X_ThreadProc() = 0;

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

template<typename ThreadProcT>
class Thread::X_ConcreteThread : public Thread {
private:
	ThreadProcT x_fnProc;

public:
	template<typename T>
	X_ConcreteThread(T &&fnProc, bool bSuspended)
		: x_fnProc(std::forward<T>(fnProc))
	{
		Thread::X_Initialize(bSuspended);
	}

private:
	void X_ThreadProc() override {
		x_fnProc();
	}
};

template<typename ThreadProcT>
IntrusivePtr<Thread> Thread::Create(ThreadProcT &&fnProc, bool bSuspended){
	return MakeIntrusive<X_ConcreteThread<std::decay_t<ThreadProcT>>>(std::forward<ThreadProcT>(fnProc), bSuspended);
}

}

#endif
