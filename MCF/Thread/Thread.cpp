// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "../../MCFCRT/env/eh_top.h"
#include "../Core/Exception.hpp"
#include "../Utilities/Assert.hpp"

namespace MCF {

void Thread::X_ThreadCloser::operator()(Handle hThread) const noexcept {
	::_MCFCRT_CloseThread(hThread);
}

Thread::X_AbstractControlBlock::~X_AbstractControlBlock(){
}

void Thread::X_AbstractControlBlock::SpawnThread(bool bSuspended){
	MCF_ASSERT(!x_hThread);

	struct Helper {
		__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
		static DWORD ThreadProc(LPVOID pParam){
			const auto pThis = IntrusivePtr<X_AbstractControlBlock>(static_cast<X_AbstractControlBlock *>(pParam));

			__MCFCRT_EH_TOP_BEGIN
			{
				pThis->X_ThreadProc();
			}
			__MCFCRT_EH_TOP_END

			return 0;
		}
	};

	if(!x_hThread.Reset(::_MCFCRT_CreateNativeThread(&Helper::ThreadProc, this, true, &x_uTid))){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"_MCFCRT_CreateThread() 失败。"));
	}
	AddRef();

	if(!bSuspended){
		::_MCFCRT_ResumeThread(x_hThread.Get());
	}
}

std::uintptr_t Thread::GetCurrentId() noexcept {
	return ::_MCFCRT_GetCurrentThreadId();
}

void Thread::Sleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	::_MCFCRT_Sleep(u64UntilFastMonoClock);
}
bool Thread::AlertableSleep(std::uint64_t u64UntilFastMonoClock) noexcept {
	return ::_MCFCRT_AlertableSleep(u64UntilFastMonoClock);
}
void Thread::AlertableSleep() noexcept {
	::_MCFCRT_AlertableSleepForever();
}
void Thread::YieldExecution() noexcept {
	::_MCFCRT_YieldThread();
}

bool Thread::Wait(std::uint64_t u64UntilFastMonoClock) const noexcept {
	MCF_ASSERT(x_pControlBlock);

	return ::_MCFCRT_WaitForThread(x_pControlBlock->GetHandle(), u64UntilFastMonoClock);
}
void Thread::Wait() const noexcept {
	MCF_ASSERT(x_pControlBlock);

	::_MCFCRT_WaitForThreadForever(x_pControlBlock->GetHandle());
}

void Thread::Suspend() noexcept {
	MCF_ASSERT(x_pControlBlock);

	::_MCFCRT_SuspendThread(x_pControlBlock->GetHandle());
}
void Thread::Resume() noexcept {
	MCF_ASSERT(x_pControlBlock);

	::_MCFCRT_ResumeThread(x_pControlBlock->GetHandle());
}

}
