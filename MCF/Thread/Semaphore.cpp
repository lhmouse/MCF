// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

class SemaphoreDelegate : CONCRETE(Semaphore) {
private:
	Impl::UniqueWinHandle xm_hSemaphore;

public:
	SemaphoreDelegate(unsigned long ulInitCount, const wchar_t *pwszName){
		if(!xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, (long)ulInitCount, LONG_MAX, pwszName))){
			DEBUG_THROW(SystemError, "CreateSemaphoreW");
		}
	}

public:
	unsigned long WaitTimeout(unsigned long long ullMilliSeconds, unsigned long ulWaitCount) noexcept {
		unsigned long ulSucceeded = 0;
		WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), dwRemaining);
				if(dwResult == WAIT_FAILED){
					ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
				}
				if(dwResult == WAIT_TIMEOUT){
					return false;
				}
				return ++ulSucceeded >= ulWaitCount;
			},
			ullMilliSeconds
		);
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
std::unique_ptr<Semaphore> Semaphore::Create(unsigned long ulInitCount, const wchar_t *pwszName){
	return std::make_unique<SemaphoreDelegate>(ulInitCount, pwszName);
}
std::unique_ptr<Semaphore> Semaphore::Create(unsigned long ulInitCount, const WideString &wsName){
	return Create(ulInitCount, wsName.GetCStr());
}

// 其他非静态成员函数。
unsigned long Semaphore::WaitTimeout(unsigned long long ullMilliSeconds, unsigned long ulWaitCount) noexcept {
	ASSERT(dynamic_cast<SemaphoreDelegate *>(this));

	return static_cast<SemaphoreDelegate *>(this)->WaitTimeout(ullMilliSeconds, ulWaitCount);
}
void Semaphore::Wait(unsigned long ulWaitCount) noexcept {
	WaitTimeout(WAIT_FOREVER, ulWaitCount);
}
void Semaphore::Signal(unsigned long ulSignalCount) noexcept {
	ASSERT(dynamic_cast<SemaphoreDelegate *>(this));

	static_cast<SemaphoreDelegate *>(this)->Release(ulSignalCount);
}
