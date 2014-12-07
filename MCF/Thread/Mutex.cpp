// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"
using namespace MCF;

namespace MCF {

template<>
bool Mutex::Lock::xDoTry() const noexcept {
	return xm_pOwner->Try(0);
}
template<>
void Mutex::Lock::xDoLock() const noexcept {
	xm_pOwner->Acquire();
}
template<>
void Mutex::Lock::xDoUnlock() const noexcept {
	xm_pOwner->Release();
}

}

// 构造函数和析构函数。
Mutex::Mutex(const wchar_t *pwszName)
	: xm_hMutex(
		[&]{
			UniqueWin32Handle hEvent(::CreateMutexW(nullptr, false, pwszName));
			if(!hEvent){
				DEBUG_THROW(SystemError, "CreateMutexW");
			}
			return std::move(hEvent);
		}())
{
}
Mutex::Mutex(const WideString &wsName)
	: Mutex(wsName.GetCStr())
{
}

// 其他非静态成员函数。
bool Mutex::Try(unsigned long long ullMilliSeconds) noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(xm_hMutex.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
void Mutex::Acquire() noexcept {
	const auto dwResult = ::WaitForSingleObject(xm_hMutex.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
void Mutex::Release() noexcept {
	if(!::ReleaseMutex(xm_hMutex.Get())){
		ASSERT_MSG(false, L"ReleaseMutex() 失败。");
	}
}

Mutex::Lock Mutex::TryLock() noexcept {
	Lock vLock(*this, false);
	vLock.Try();
	return std::move(vLock);
}
Mutex::Lock Mutex::GetLock() noexcept {
	return Lock(*this);
}
