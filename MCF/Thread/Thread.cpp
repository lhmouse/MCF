// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "../../MCFCRT/env/eh_top.h"
#include "../../MCFCRT/env/fenv.h"
#include "../Core/Exception.hpp"

namespace MCF {

void Thread::X_ThreadCloser::operator()(Thread::Handle hThread) const noexcept {
	::MCFCRT_CloseThread(hThread);
}

// 静态成员函数。
std::uintptr_t Thread::GetCurrentId() noexcept {
	return ::MCFCRT_GetCurrentThreadId();
}

void Thread::Sleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	::MCFCRT_Sleep(u64UntilFastMonoClock);
}
bool Thread::AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	return ::MCFCRT_AlertableSleep(u64UntilFastMonoClock);
}
void Thread::AlertableSleep() noexcept {
	::MCFCRT_AlertableSleepInfinitely();
}
void Thread::YieldExecution() noexcept {
	::MCFCRT_YieldThread();
}

// 构造函数和析构函数。
Thread::~Thread(){
	if(x_pException){
		std::terminate();
	}
}

// 其他非静态成员函数。
void Thread::X_Initialize(bool bSuspended){
	ASSERT_MSG(!x_hThread, L"Thread 只能被初始化一次。");

	struct Helper {
		__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
		static DWORD ThreadProc(LPVOID pParam){
			const auto pThis = static_cast<Thread *>(pParam);

			__MCFCRT_EH_TOP_BEGIN
			{
				__MCFCRT_FEnvInit();

				try {
					pThis->X_ThreadProc();
				} catch(...){
					pThis->x_pException = std::current_exception();
				}
			}
			__MCFCRT_EH_TOP_END

			pThis->x_uThreadId.Store(0, kAtomicRelease);
			pThis->DropRef();

			return 0;
		}
	};

	std::uintptr_t uThreadId = 0;
	if(!x_hThread.Reset(::MCFCRT_CreateNativeThread(&Helper::ThreadProc, this, true, &uThreadId))){
		DEBUG_THROW(SystemError, "MCFCRT_CreateThread"_rcs);
	}
	AddRef();
	x_uThreadId.Store(uThreadId, kAtomicRelease);

	if(!bSuspended){
		Resume();
	}
}

bool Thread::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	return ::MCFCRT_WaitForThread(x_hThread.Get(), u64UntilFastMonoClock);
}
void Thread::Wait() const noexcept {
	::MCFCRT_WaitForThreadInfinitely(x_hThread.Get());
}

bool Thread::IsAlive() const noexcept {
	return GetId() != 0;
}
std::uintptr_t Thread::GetId() const noexcept {
	return x_uThreadId.Load(kAtomicRelaxed);
}

void Thread::Suspend() noexcept {
	::MCFCRT_SuspendThread(x_hThread.Get());
}
void Thread::Resume() noexcept {
	::MCFCRT_ResumeThread(x_hThread.Get());
}

}
