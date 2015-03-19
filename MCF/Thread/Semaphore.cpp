// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

// 构造函数和析构函数。
Semaphore::Semaphore(std::size_t uInitCount, const wchar_t *pwszName)
	: x_hSemaphore(
		[&]{
			UniqueWin32Handle hSemaphore(::CreateSemaphoreW(nullptr, (long)uInitCount, LONG_MAX, pwszName));
			if(!hSemaphore){
				DEBUG_THROW(SystemError, "CreateSemaphoreW");
			}
			return hSemaphore;
		}())
{
}
Semaphore::Semaphore(std::size_t uInitCount, const WideString &wsName)
	: Semaphore(uInitCount, wsName.GetStr())
{
}

// 其他非静态成员函数。
std::size_t Semaphore::Wait(unsigned long long ullMilliSeconds) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
	const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
std::size_t Semaphore::Post(std::size_t uPostCount) noexcept {
	long lPrevCount;
	if(!::ReleaseSemaphore(x_hSemaphore.Get(), (long)uPostCount, &lPrevCount)){
		ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
	}
	return (std::size_t)lPrevCount;
}

std::size_t Semaphore::BatchWait(unsigned long long ullMilliSeconds, std::size_t uWaitCount) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	std::size_t uSucceeded = 0;
	auto ullTimeRemaining = ullMilliSeconds;
	while(uSucceeded < uWaitCount){
		const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		if(dwResult != WAIT_TIMEOUT){
			++uSucceeded;
			continue;
		}
		const auto ullNow = GetFastMonoClock();
		if(ullUntil <= ullNow){
			break;
		}
		ullTimeRemaining = ullUntil - ullNow;
	}
	return uSucceeded;
}
void Semaphore::BatchWait(std::size_t uWaitCount) noexcept {
	std::size_t uSucceeded = 0;
	while(uSucceeded < uWaitCount){
		const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), INFINITE);
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		++uSucceeded;
	}
}

}
