// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_THREAD_HPP_
#define MCF_THREAD_THREAD_HPP_

#include "../Core/UniqueHandle.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include <MCFCRT/env/thread.h>
#include <type_traits>
#include <utility>
#include <cstdint>

namespace MCF {

inline std::uintptr_t GetCurrentThreadId() noexcept {
	return ::_MCFCRT_GetCurrentThreadId();
}

inline void Sleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	::_MCFCRT_Sleep(u64UntilFastMonoClock);
}
inline bool AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	return ::_MCFCRT_AlertableSleep(u64UntilFastMonoClock);
}
inline void AlertableSleep() noexcept {
	::_MCFCRT_AlertableSleepForever();
}
inline void YieldThread() noexcept {
	::_MCFCRT_YieldThread();
}

namespace Impl_Thread {
	using Handle = ::_MCFCRT_ThreadHandle;

	struct ThreadCloser {
		constexpr Handle operator()() const noexcept {
			return nullptr;
		}
		void operator()(Handle hThread) const noexcept {
			::_MCFCRT_CloseThread(hThread);
		}
	};
}

class Thread : public IntrusiveBase<Thread> {
public:
	using Handle = Impl_Thread::Handle;

private:
	UniqueHandle<Impl_Thread::ThreadCloser> x_hThread;
	std::uintptr_t x_uThreadId = 0;

protected:
	constexpr Thread() noexcept { }

public:
	virtual ~Thread();

protected:
	virtual void X_ThreadProc() const = 0;

	void X_Spawn(bool bSuspended);

public:
	Handle GetHandle() const noexcept {
		return x_hThread.Get();
	}
	std::uintptr_t GetThreadId() const noexcept {
		return x_uThreadId;
	}

	bool Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
		return ::_MCFCRT_WaitForThread(x_hThread.Get(), u64UntilFastMonoClock);
	}
	void Wait() const noexcept {
		::_MCFCRT_WaitForThreadForever(x_hThread.Get());
	}

	void Suspend() noexcept {
		::_MCFCRT_SuspendThread(x_hThread.Get());
	}
	void Resume() noexcept {
		::_MCFCRT_ResumeThread(x_hThread.Get());
	}
};

extern template class IntrusivePtr<Thread>;

namespace Impl_Thread {
	template<typename FunctionT>
	class ConcreteThread final : public Thread {
	private:
		std::decay_t<FunctionT> x_vFunction;

	public:
		ConcreteThread(FunctionT &vFunction, bool bSuspended)
			: x_vFunction(std::forward<FunctionT>(vFunction))
		{
			X_Spawn(bSuspended);
		}
		~ConcreteThread();

	protected:
		void X_ThreadProc() const override {
			std::forward<FunctionT>(x_vFunction)();
		}
	};

	template<typename FunctionT>
	ConcreteThread<FunctionT>::~ConcreteThread(){ }
}

template<typename FunctionT>
IntrusivePtr<Impl_Thread::ConcreteThread<FunctionT>> MakeThread(FunctionT &&vFunction, bool bSuspended = false){
	return MakeIntrusive<Impl_Thread::ConcreteThread<FunctionT>>(vFunction, bSuspended);
}

}

#endif
