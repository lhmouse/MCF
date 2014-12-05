// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include "../../MCFCRT/env/thread.h"
using namespace MCF;

namespace {

class ThreadDelegate : CONCRETE(Thread) {
private:
	std::function<void ()> xm_fnProc;
	Impl::UniqueWinHandle xm_hThread;
	volatile unsigned long xm_ulThreadId;
	std::exception_ptr xm_pException;

public:
	explicit ThreadDelegate(std::function<void ()> &&fnProc)
		: xm_fnProc(std::move(fnProc))
	{
		unsigned long ulThreadId;
		if(!xm_hThread.Reset(::MCF_CRT_CreateThread(
			[](std::intptr_t nParam) noexcept -> unsigned {
				const auto pThis = reinterpret_cast<ThreadDelegate *>(nParam);
				try {
					pThis->xm_fnProc();
				} catch(...){
					pThis->xm_pException = std::current_exception();
				}
				__atomic_store_n(&(pThis->xm_ulThreadId), 0, __ATOMIC_RELEASE);
				pThis->DropRef();
				return 0;
			},
			reinterpret_cast<std::intptr_t>(this), CREATE_SUSPENDED, &ulThreadId)))
		{
			DEBUG_THROW(SystemError, "MCF_CRT_CreateThread");
		}
		AddRef();
		__atomic_store_n(&xm_ulThreadId, 0, __ATOMIC_RELAXED);
	}

public:
	bool WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
		return WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				return ::WaitForSingleObject(xm_hThread.Get(), dwRemaining) != WAIT_TIMEOUT;
			},
			ullMilliSeconds
		);
	}
	std::exception_ptr JoinNoThrow() const noexcept {
		WaitTimeout(WAIT_FOREVER);
		return xm_pException; // 不要 move()。
	}

	unsigned long GetId() const noexcept {
		return __atomic_load_n(&xm_ulThreadId, __ATOMIC_ACQUIRE);
	}

	void Suspend() noexcept {
		::SuspendThread(xm_hThread.Get());
	}
	void Resume() noexcept {
		::ResumeThread(xm_hThread.Get());
	}
};

}

// 静态成员函数。
unsigned long Thread::GetCurrentId() noexcept {
	return ::GetCurrentThreadId();
}

IntrusivePtr<Thread> Thread::Create(std::function<void ()> fnProc, bool bSuspended){
	IntrusivePtr<Thread> pThread(new ThreadDelegate(std::move(fnProc)));
	if(!bSuspended){
		pThread->Resume();
	}
	return std::move(pThread);
}

// 其他非静态成员函数。
bool Thread::WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
	ASSERT(dynamic_cast<const ThreadDelegate *>(this));

	return static_cast<const ThreadDelegate *>(this)->WaitTimeout(ullMilliSeconds);
}
void Thread::Wait() const noexcept {
	WaitTimeout(WAIT_FOREVER);
}

std::exception_ptr Thread::JoinNoThrow() const noexcept {
	ASSERT(dynamic_cast<const ThreadDelegate *>(this));

	return static_cast<const ThreadDelegate *>(this)->JoinNoThrow();
}
void Thread::Join() const {
	const auto pException = JoinNoThrow();
	if(pException){
		std::rethrow_exception(pException);
	}
}

bool Thread::IsAlive() const noexcept {
	return !WaitTimeout(0);
}
unsigned long Thread::GetId() const noexcept {
	ASSERT(dynamic_cast<const ThreadDelegate *>(this));

	return static_cast<const ThreadDelegate *>(this)->GetId();
}

void Thread::Suspend() noexcept {
	ASSERT(dynamic_cast<ThreadDelegate *>(this));

	static_cast<ThreadDelegate *>(this)->Suspend();
}
void Thread::Resume() noexcept {
	ASSERT(dynamic_cast<ThreadDelegate *>(this));

	static_cast<ThreadDelegate *>(this)->Resume();
}
