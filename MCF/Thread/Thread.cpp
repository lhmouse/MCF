// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "Atomic.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// 静态成员函数。
std::size_t Thread::GetCurrentId() noexcept {
	return ::GetCurrentThreadId();
}

IntrusivePtr<Thread> Thread::Create(std::function<void ()> fnProc, bool bSuspended){
	return IntrusivePtr<Thread>(new Thread(std::move(fnProc), bSuspended));
}

// 构造函数和析构函数。
Thread::Thread(std::function<void ()> fnProc, bool bSuspended)
	: x_fnProc(std::move(fnProc))
{
	const auto ThreadProc = [](std::intptr_t nParam) noexcept -> unsigned {
		const auto pThis = (Thread *)nParam;
		try {
			pThis->x_fnProc();
		} catch(...){
			pThis->x_pException = std::current_exception();
		}
		AtomicStore(pThis->x_ulThreadId, 0, MemoryModel::RELEASE);
		pThis->DropRef();
		return 0;
	};

	unsigned long ulThreadId;
	if(!x_hThread.Reset(::MCF_CRT_CreateThread(ThreadProc, (std::intptr_t)this, CREATE_SUSPENDED, &ulThreadId))){
		DEBUG_THROW(SystemError, "MCF_CRT_CreateThread");
	}
	AddRef();
	AtomicStore(x_ulThreadId, ulThreadId, MemoryModel::RELEASE);

	if(!bSuspended){
		Resume();
	}
}

bool Thread::Wait(unsigned long long ullMilliSeconds) const noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hThread.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
	const auto dwResult = ::WaitForSingleObject(x_hThread.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}

std::exception_ptr Thread::JoinNoThrow() const noexcept {
	Wait();
	return x_pException; // 不要 move()。
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
	return AtomicLoad(x_ulThreadId, MemoryModel::ACQUIRE);
}

void Thread::Suspend() noexcept {
	if(::SuspendThread(x_hThread.Get()) == (DWORD)-1){
		ASSERT_MSG(false, L"SuspendThread() 失败。");
	}
}
void Thread::Resume() noexcept {
	if(::ResumeThread(x_hThread.Get()) == (DWORD)-1){
		ASSERT_MSG(false, L"ResumeThread() 失败。");
	}
}

}
