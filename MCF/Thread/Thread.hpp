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

public:
	explicit Thread(bool bSuspended);
	~Thread(); // 如果有被捕获的异常，调用 std::terminate()。

private:
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

namespace Impl_Thread {
	template<typename ThreadProcT>
	class ConcreteThread : public Thread {
	private:
		std::remove_reference_t<ThreadProcT> x_fnProc;

	public:
		ConcreteThread(bool bSuspended, ThreadProcT &&fnProc)
			: Thread(bSuspended)
			, x_fnProc(std::forward<ThreadProcT>(fnProc))
		{
		}
		~ConcreteThread(){
		}

	private:
		void X_ThreadProc() override {
			x_fnProc();
		}
	};
}

template<typename ThreadProcT>
IntrusivePtr<Thread> Thread::Create(ThreadProcT &&fnProc, bool bSuspended){
	return IntrusivePtr<Thread>(new Impl_Thread::ConcreteThread<ThreadProcT>(bSuspended, std::forward<ThreadProcT>(fnProc)));
}

}

#endif
