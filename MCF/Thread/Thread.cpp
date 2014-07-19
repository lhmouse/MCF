// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

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
	static unsigned int xThreadProc(std::intptr_t nParam) noexcept {
		auto *const pThis = (ThreadDelegate *)nParam;
		try {
			const std::shared_ptr<ThreadDelegate> pInstance(std::move(pThis->xm_pLock));
			pThis->xm_pLock.reset(); // 打破循环引用。

			pThis->xm_fnProc();
		} catch(...){
			pThis->xm_pException = std::current_exception();
		}
		pThis->xm_ulThreadId = 0;
		return 0;
	}

public:
	static std::shared_ptr<ThreadDelegate> Create(std::function<void ()> &&fnProc, bool bSuspended);

private:
	std::shared_ptr<ThreadDelegate> xm_pLock;
	std::function<void ()> xm_fnProc;
	Impl::UniqueWinHandle xm_hThread;
	unsigned long xm_ulThreadId;
	std::exception_ptr xm_pException;

private:
	explicit ThreadDelegate(std::function<void ()> &&fnProc) noexcept
		: xm_fnProc(std::move(fnProc))
	{
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
	std::exception_ptr JoinNoThrow() const {
		WaitTimeout(WAIT_FOREVER);
		return xm_pException; // 不要 move()。
	}

	unsigned long GetId() const noexcept {
		return xm_ulThreadId;
	}

	void Suspend() noexcept {
		::SuspendThread(xm_hThread.Get());
	}
	void Resume() noexcept {
		::ResumeThread(xm_hThread.Get());
	}
};

inline std::shared_ptr<ThreadDelegate> ThreadDelegate::Create(std::function<void ()> &&fnProc, bool bSuspended){
	struct Helper : public ThreadDelegate {
		explicit Helper(std::function<void ()> &&fnProc)
			: ThreadDelegate(std::move(fnProc))
		{
		}
	};
	auto pRet(std::make_shared<Helper>(std::move(fnProc)));

	pRet->xm_hThread.Reset(::MCF_CRT_CreateThread(&xThreadProc, (std::intptr_t)pRet.get(), CREATE_SUSPENDED, &pRet->xm_ulThreadId));
	if(!pRet->xm_hThread){
		MCF_THROW(::GetLastError(), L"MCF_CRT_CreateThread() 失败。"_wso);
	}
	pRet->xm_pLock = pRet; // 制造循环引用。这样代理对象就不会被删掉。
	if(!bSuspended){
		pRet->Resume();
	}
	return std::move(pRet);
}

}

// 静态成员函数。
unsigned long Thread::GetCurrentId() noexcept {
	return ::GetCurrentThreadId();
}

std::shared_ptr<Thread> Thread::Create(std::function<void ()> fnProc, bool bSuspended){
	return ThreadDelegate::Create(std::move(fnProc), bSuspended);
}

// 其他非静态成员函数。
bool Thread::WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
	ASSERT(dynamic_cast<const ThreadDelegate *>(this));

	return ((const ThreadDelegate *)this)->WaitTimeout(ullMilliSeconds);
}
void Thread::Wait() const noexcept {
	WaitTimeout(WAIT_FOREVER);
}

std::exception_ptr Thread::JoinNoThrow() const {
	ASSERT(dynamic_cast<const ThreadDelegate *>(this));

	return ((const ThreadDelegate *)this)->JoinNoThrow();
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

	return ((const ThreadDelegate *)this)->GetId();
}

void Thread::Suspend() noexcept {
	ASSERT(dynamic_cast<ThreadDelegate *>(this));

	static_cast<ThreadDelegate *>(this)->Suspend();
}
void Thread::Resume() noexcept {
	ASSERT(dynamic_cast<ThreadDelegate *>(this));

	static_cast<ThreadDelegate *>(this)->Resume();
}
