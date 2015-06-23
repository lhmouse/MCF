// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Semaphore.hpp"
#include "_WaitForSingleObject64.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"

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
	return WaitForSingleObject64(x_hSemaphore.Get(), &ullMilliSeconds);
}
void Semaphore::Wait() noexcept {
	WaitForSingleObject64(x_hSemaphore.Get(), nullptr);
}
std::size_t Semaphore::Post(std::size_t uPostCount) noexcept {
	long lPrevCount;
	if(!::ReleaseSemaphore(x_hSemaphore.Get(), (long)uPostCount, &lPrevCount)){
		ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
	}
	return (std::size_t)lPrevCount;
}

std::size_t Semaphore::BatchWait(unsigned long long ullMilliSeconds, std::size_t uWaitCount) noexcept {
	std::size_t uSucceeded = 0;
	while(uSucceeded < uWaitCount){
		if(!WaitForSingleObject64(x_hSemaphore.Get(), &ullMilliSeconds)){
			break;
		}
		++uSucceeded;
	}
	return uSucceeded;
}
void Semaphore::BatchWait(std::size_t uWaitCount) noexcept {
	std::size_t uSucceeded = 0;
	while(uSucceeded < uWaitCount){
		WaitForSingleObject64(x_hSemaphore.Get(), nullptr);
	}
}

}
