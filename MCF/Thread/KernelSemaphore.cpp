// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelSemaphore.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace {
	UniqueWin32Handle CheckedCreateSemaphore(std::size_t uInitCount, const wchar_t *pwszName){
		if(uInitCount >= static_cast<std::size_t>(LONG_MAX)){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "Initial count for a kernel semaphore is too large"_rcs);
		}
		UniqueWin32Handle hSemaphore(::CreateSemaphoreW(nullptr, static_cast<long>(uInitCount), LONG_MAX, pwszName));
		if(!hSemaphore){
			DEBUG_THROW(SystemError, "CreateSemaphoreW"_rcs);
		}
		return hSemaphore;
	}
}

// 构造函数和析构函数。
KernelSemaphore::KernelSemaphore(std::size_t uInitCount, const wchar_t *pwszName)
	: x_hSemaphore(CheckedCreateSemaphore(uInitCount, pwszName))
{
}
KernelSemaphore::KernelSemaphore(std::size_t uInitCount, const WideString &wsName)
	: KernelSemaphore(uInitCount, wsName.GetStr())
{
}

// 其他非静态成员函数。
bool KernelSemaphore::Wait(std::uint64_t u64MilliSeconds) noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), Min(u64Until - u64Now, 0x7FFFFFFFu));
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		if(dwResult != WAIT_TIMEOUT){
			return true;
		}
		u64Now = GetFastMonoClock();
		if(u64Until <= u64Now){
			return false;
		}
	}
}
void KernelSemaphore::Wait() noexcept {
	const auto dwResult = ::WaitForSingleObject(x_hSemaphore.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
std::size_t KernelSemaphore::Post(std::size_t uPostCount) noexcept {
	long lPrevCount;
	if(!::ReleaseSemaphore(x_hSemaphore.Get(), static_cast<long>(uPostCount), &lPrevCount)){
		ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
	}
	return static_cast<std::size_t>(lPrevCount);
}

}
