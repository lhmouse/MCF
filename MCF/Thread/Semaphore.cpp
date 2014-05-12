// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "../Core/Exception.hpp"
#include "../Core/UniqueHandle.hpp"
using namespace MCF;

namespace {

class SemaphoreDelegate : CONCRETE(Semaphore) {
private:
	struct xSemaphoreCloser {
		constexpr HANDLE operator()() const noexcept {
			return NULL;
		}
		void operator()(HANDLE hSemaphore) const noexcept {
			::CloseHandle(hSemaphore);
		}
	};

private:
	UniqueHandle<xSemaphoreCloser> xm_hSemaphore;

public:
	SemaphoreDelegate(unsigned long ulInitCount, unsigned long ulMaxCount, const WideStringObserver &wsoName){
		if(wsoName.IsEmpty()){
			xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, (long)ulInitCount, (long)ulMaxCount, nullptr));
		} else {
			xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, (long)ulInitCount, (long)ulMaxCount, wsoName.GetNullTerminated<MAX_PATH>().GetData()));
		}
		if(!xm_hSemaphore){
			MCF_THROW(::GetLastError(), L"CreateSemaphoreW() 失败。");
		}
	}

public:
	unsigned long WaitTimeout(unsigned long ulMilliSeconds, unsigned long ulWaitCount) noexcept {
		unsigned long ulSucceeded = 0;
		if(ulMilliSeconds == INFINITE){
			for(auto i = ulWaitCount; i; --i){
				::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
				++ulSucceeded;
			}
		} else {
			const auto ulWaitUntil = ::GetTickCount() + ulMilliSeconds;
			auto ulTimeToWait = ulMilliSeconds;
			for(auto i = ulWaitCount; i; --i){
				if(::WaitForSingleObject(xm_hSemaphore.Get(), ulTimeToWait) == WAIT_TIMEOUT){
					break;
				}
				++ulSucceeded;
				const auto ulCurrent = ::GetTickCount();
				if(ulWaitUntil <= ulCurrent){
					break;
				}
				ulTimeToWait = ulWaitUntil - ulCurrent;
			}
		}
		return ulSucceeded;
	}
	void Release(unsigned long ulSignalCount) noexcept {
		if(!::ReleaseSemaphore(xm_hSemaphore.Get(), (long)ulSignalCount, nullptr)){
			ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<Semaphore> Semaphore::Create(unsigned long ulInitCount, unsigned long ulMaxCount, const WideStringObserver &wsoName){
	return std::make_unique<SemaphoreDelegate>(ulInitCount, ulMaxCount, wsoName);
}

// 其他非静态成员函数。
unsigned long Semaphore::WaitTimeout(unsigned long ulMilliSeconds, unsigned long ulWaitCount) noexcept {
	ASSERT(dynamic_cast<SemaphoreDelegate *>(this));

	return ((SemaphoreDelegate *)this)->WaitTimeout(ulMilliSeconds, ulWaitCount);
}
void Semaphore::Wait(unsigned long ulWaitCount) noexcept {
	WaitTimeout(INFINITE, ulWaitCount);
}
void Semaphore::Signal(unsigned long ulSignalCount) noexcept {
	ASSERT(dynamic_cast<SemaphoreDelegate *>(this));

	((SemaphoreDelegate *)this)->Release(ulSignalCount);
}
