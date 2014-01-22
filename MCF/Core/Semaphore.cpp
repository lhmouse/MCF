// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
using namespace MCF;

// 嵌套类定义。
class Semaphore::xDelegate : NO_COPY {
private:
	struct xSemaphoreCloser {
		constexpr HANDLE operator()() const {
			return NULL;
		}
		void operator()(HANDLE hSemaphore) const {
			::CloseHandle(hSemaphore);
		}
	};
private:
	UniqueHandle<HANDLE, xSemaphoreCloser> xm_hSemaphore;
public:
	xDelegate(long lInitCount, long lMaxCount){
		UniqueHandle<HANDLE, xSemaphoreCloser> hSemaphore(::CreateSemaphoreW(nullptr, lInitCount, lMaxCount, nullptr));
		if(!hSemaphore){
			MCF_THROW(::GetLastError(), L"CreateSemaphoreW() 失败。");
		}
		xm_hSemaphore = std::move(hSemaphore);
	}
public:
	HANDLE GetHandle() const noexcept {
		return xm_hSemaphore;
	}
};

// 静态成员函数。
void Semaphore::xUnlock(Semaphore::xDelegate *pDelegate) noexcept {
	if(pDelegate){
		::ReleaseSemaphore(pDelegate->GetHandle(), 1, nullptr);
	}
}

// 构造函数和析构函数。
Semaphore::Semaphore(long lInitCount, long lMaxCount)
	: xm_pDelegate(new xDelegate(lInitCount, lMaxCount))
{
}
Semaphore::~Semaphore(){
}

// 其他非静态成员函数。
Semaphore::LockHolder Semaphore::WaitTimeOut(unsigned long ulMilliSeconds) noexcept {
	if(::WaitForSingleObject(xm_pDelegate->GetHandle(), ulMilliSeconds) == WAIT_TIMEOUT){
		return nullptr;
	}
	return LockHolder(xm_pDelegate.get());
}
MCF::VVector<Semaphore::LockHolder> Semaphore::WaitTimeOut(long lWaitCount, unsigned long ulMilliSeconds) noexcept {
	MCF::VVector<Semaphore::LockHolder> vecRet;
	if(ulMilliSeconds == INFINITE){
		for(long i = 0; i < lWaitCount; ++i){
			::WaitForSingleObject(xm_pDelegate->GetHandle(), INFINITE);
			vecRet.Push(xm_pDelegate.get());
		}
	} else {
		const auto ulWaitUntil = ::GetTickCount() + ulMilliSeconds;
		unsigned long ulTimeToWait = ulMilliSeconds;
		for(long i = 0; i < lWaitCount; ++i){
			if(::WaitForSingleObject(xm_pDelegate->GetHandle(), ulTimeToWait) == WAIT_TIMEOUT){
				break;
			}
			vecRet.Push(xm_pDelegate.get());
			ulTimeToWait = ulWaitUntil - ::GetTickCount();
		}
	}
	return std::move(vecRet);
}
Semaphore::LockHolder Semaphore::Wait() noexcept {
	return WaitTimeOut(INFINITE);
}
MCF::VVector<Semaphore::LockHolder> Semaphore::Wait(long lWaitCount) noexcept {
	return WaitTimeOut(lWaitCount, INFINITE);
}
