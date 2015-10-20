// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "../Core/Exception.hpp"

namespace MCF {

void Thread::X_ThreadCloser::operator()(void *hThread) const noexcept {
	::MCF_CRT_CloseThread(hThread);
}

// 静态成员函数。
IntrusivePtr<Thread> Thread::Create(Function<void ()> fnProc, bool bSuspended){
	return IntrusivePtr<Thread>(new Thread(std::move(fnProc), bSuspended));
}

unsigned Thread::GetCurrentId() noexcept {
	return ::MCF_CRT_GetCurrentThreadId();
}

void Thread::Sleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	::MCF_CRT_Sleep(u64UntilFastMonoClock);
}
bool Thread::AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	return ::MCF_CRT_AlertableSleep(u64UntilFastMonoClock);
}
void Thread::AlertableSleep() noexcept {
	::MCF_CRT_AlertableSleepInfinitely();
}
void Thread::YieldExecution() noexcept {
	::MCF_CRT_YieldThread();
}

// 构造函数和析构函数。
Thread::Thread(Function<void ()> fnProc, bool bSuspended)
	: x_fnProc(std::move(fnProc))
{
	const auto ThreadProc = [](std::intptr_t nParam) noexcept -> unsigned {
		const auto pThis = (Thread *)nParam;
		try {
			pThis->x_fnProc();
		} catch(...){
			pThis->x_pException = std::current_exception();
		}
		pThis->x_uThreadId.Store(0, kAtomicRelease);
		pThis->DropRef();
		return 0;
	};

	unsigned uThreadId = 0;
	if(!x_hThread.Reset(::MCF_CRT_CreateThread(ThreadProc, (std::intptr_t)this, CREATE_SUSPENDED, &uThreadId))){
		DEBUG_THROW(SystemError, "MCF_CRT_CreateThread"_rcs);
	}
	AddRef();
	x_uThreadId.Store(uThreadId, kAtomicRelease);

	if(!bSuspended){
		Resume();
	}
}
Thread::~Thread(){
	if(x_pException){
		std::terminate();
	}
}

bool Thread::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	return ::MCF_CRT_WaitForThread(x_hThread.Get(), u64UntilFastMonoClock);
}
void Thread::Wait() const noexcept {
	::MCF_CRT_WaitForThreadInfinitely(x_hThread.Get());
}

bool Thread::IsAlive() const noexcept {
	return GetId() != 0;
}
unsigned Thread::GetId() const noexcept {
	return x_uThreadId.Load(kAtomicRelaxed);
}

void Thread::Suspend() noexcept {
	::MCF_CRT_SuspendThread(x_hThread.Get());
}
void Thread::Resume() noexcept {
	::MCF_CRT_ResumeThread(x_hThread.Get());
}

}
