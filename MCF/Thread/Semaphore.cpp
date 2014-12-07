// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"
using namespace MCF;

// 构造函数和析构函数。
Semaphore::Semaphore(unsigned long ulInitCount, const wchar_t *pwszName)
	: xm_hSemaphore(
		[&]{
			UniqueWin32Handle hSemaphore(::CreateSemaphoreW(nullptr, (long)ulInitCount, LONG_MAX, pwszName));
			if(!hSemaphore){
				DEBUG_THROW(SystemError, "CreateSemaphoreW");
			}
			return std::move(hSemaphore);
		}())
{
}
Semaphore::Semaphore(unsigned long ulInitCount, const WideString &wsName)
	: Semaphore(ulInitCount, wsName.GetCStr())
{
}

// 其他非静态成员函数。
unsigned long Semaphore::Wait(unsigned long long ullMilliSeconds) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
void Semaphore::Wait() noexcept {
	const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
unsigned long Semaphore::Post(unsigned long ulPostCount) noexcept {
	long lPrevCount;
	if(!::ReleaseSemaphore(xm_hSemaphore.Get(), (long)ulPostCount, &lPrevCount)){
		ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
	}
	return (unsigned long)lPrevCount;
}

unsigned long Semaphore::BatchWait(unsigned long long ullMilliSeconds, unsigned long ulWaitCount) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	unsigned long ulSucceeded = 0;
	auto ullTimeRemaining = ullMilliSeconds;
	while(ulSucceeded < ulWaitCount){
		const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		if(dwResult != WAIT_TIMEOUT){
			++ulSucceeded;
			continue;
		}
		const auto ullNow = GetFastMonoClock();
		if(ullUntil <= ullNow){
			break;
		}
		ullTimeRemaining = ullUntil - ullNow;
	}
	return ulSucceeded;
}
void Semaphore::BatchWait(unsigned long ulWaitCount) noexcept {
	unsigned long ulSucceeded = 0;
	while(ulSucceeded < ulWaitCount){
		const auto dwResult = ::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		++ulSucceeded;
	}
}
