// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"
using namespace MCF;

// 静态成员函数。
std::size_t Thread::GetCurrentId() noexcept {
	return ::GetCurrentThreadId();
}

IntrusivePtr<Thread> Thread::Create(std::function<void ()> fnProc, bool bSuspended){
	IntrusivePtr<Thread> pThread(new Thread(std::move(fnProc)));
	if(!bSuspended){
		pThread->Resume();
	}
	return std::move(pThread);
}

// 构造函数和析构函数。
Thread::Thread(std::function<void ()> fnProc)
	: xm_fnProc(std::move(fnProc))
{
	const auto ThreadProc = [](std::intptr_t nParam) noexcept -> unsigned {
		const auto pThis = reinterpret_cast<Thread *>(nParam);
		try {
			pThis->xm_fnProc();
		} catch(...){
			pThis->xm_pException = std::current_exception();
		}
		__atomic_store_n(&(pThis->xm_ulThreadId), 0, __ATOMIC_RELEASE);
		pThis->DropRef();
		return 0;
	};

	unsigned long ulThreadId;
	if(!xm_hThread.Reset(::MCF_CRT_CreateThread(
		ThreadProc, reinterpret_cast<std::intptr_t>(this), CREATE_SUSPENDED, &ulThreadId)))
	{
		DEBUG_THROW(SystemError, "MCF_CRT_CreateThread");
	}
	AddRef();
	__atomic_store_n(&xm_ulThreadId, 0, __ATOMIC_RELEASE);
}

bool Thread::Wait(unsigned long long ullMilliSeconds) const noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(xm_hThread.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		if(dwResult != WAIT_TIMEOUT){
			bResult = true;
			break;
		}
		const auto ullNow = GetFastMonoClock();
		if(ullUntil <= ullNow){
			break;
		}
		ullTimeRemaining = ullUntil - ullNow;
	}
	return bResult;
}
void Thread::Wait() const noexcept {
	const auto dwResult = ::WaitForSingleObject(xm_hThread.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}

std::exception_ptr Thread::JoinNoThrow() const noexcept {
	Wait();
	return xm_pException; // 不要 move()。
}
void Thread::Join() const {
	const auto pException = JoinNoThrow();
	if(pException){
		std::rethrow_exception(pException);
	}
}

bool Thread::IsAlive() const noexcept {
	return GetId() != 0;
}
std::size_t Thread::GetId() const noexcept {
	return __atomic_load_n(&xm_ulThreadId, __ATOMIC_ACQUIRE);
}

void Thread::Suspend() noexcept {
	if(::SuspendThread(xm_hThread.Get()) == (DWORD)-1){
		ASSERT_MSG(false, L"SuspendThread() 失败。");
	}
}
void Thread::Resume() noexcept {
	if(::ResumeThread(xm_hThread.Get()) == (DWORD)-1){
		ASSERT_MSG(false, L"ResumeThread() 失败。");
	}
}
