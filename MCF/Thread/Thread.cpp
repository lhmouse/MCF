// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/thread.h"
#include "WaitForSingleObject64.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Time.hpp"

namespace MCF {

// 静态成员函数。
std::size_t Thread::GetCurrentId() noexcept {
	return ::GetCurrentThreadId();
}

IntrusivePtr<Thread> Thread::Create(Function<void ()> fnProc, bool bSuspended){
	return IntrusivePtr<Thread>(new Thread(std::move(fnProc), bSuspended));
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
		pThis->x_ulThreadId.Store(0, MemoryModel::kRelease);
		pThis->DropRef();
		return 0;
	};

	unsigned long ulThreadId;
	if(!x_hThread.Reset(::MCF_CRT_CreateThread(ThreadProc, (std::intptr_t)this, CREATE_SUSPENDED, &ulThreadId))){
		DEBUG_THROW(SystemError, "MCF_CRT_CreateThread");
	}
	AddRef();
	x_ulThreadId.Store(ulThreadId, MemoryModel::kRelease);

	if(!bSuspended){
		Resume();
	}
}
Thread::~Thread(){
	if(x_pException){
		std::terminate();
	}
}

bool Thread::Wait(std::uint64_t u64MilliSeconds) const noexcept {
	return WaitForSingleObject64(x_hThread.Get(), &u64MilliSeconds);
}
void Thread::Wait() const noexcept {
	WaitForSingleObject64(x_hThread.Get(), nullptr);
}

std::exception_ptr Thread::JoinNoThrow() const noexcept {
	Wait();

	return x_pException; // 不要 move()。
}
void Thread::Join(){
	Wait();

	auto pException = std::exchange(x_pException, std::exception_ptr());
	if(pException){
		std::rethrow_exception(std::move(pException));
	}
}

bool Thread::IsAlive() const noexcept {
	return GetId() != 0;
}
std::size_t Thread::GetId() const noexcept {
	return x_ulThreadId.Load(MemoryModel::kRelaxed);
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
