// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

template<>
bool KernelMutex::UniqueLock::xDoTry() const noexcept {
	return x_pOwner->Try(0);
}
template<>
void KernelMutex::UniqueLock::xDoLock() const noexcept {
	x_pOwner->Lock();
}
template<>
void KernelMutex::UniqueLock::xDoUnlock() const noexcept {
	x_pOwner->Unlock();
}

// 构造函数和析构函数。
KernelMutex::KernelMutex(const wchar_t *pwszName)
	: x_hMutex(
		[&]{
			UniqueWin32Handle hEvent(::CreateMutexW(nullptr, false, pwszName));
			if(!hEvent){
				DEBUG_THROW(SystemError, "CreateMutexW");
			}
			return hEvent;
		}())
{
}
KernelMutex::KernelMutex(const WideString &wsName)
	: KernelMutex(wsName.GetStr())
{
}

// 其他非静态成员函数。
bool KernelMutex::Try(unsigned long long ullMilliSeconds) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hMutex.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
void KernelMutex::Lock() noexcept {
	const auto dwResult = ::WaitForSingleObject(x_hMutex.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
void KernelMutex::Unlock() noexcept {
	if(!::ReleaseMutex(x_hMutex.Get())){
		ASSERT_MSG(false, L"ReleaseMutex() 失败。");
	}
}

}
