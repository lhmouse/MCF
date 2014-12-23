// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"
using namespace MCF;

// 构造函数和析构函数。
Event::Event(bool bInitSet, const wchar_t *pwszName)
	: xm_hEvent(
		[&]{
			UniqueWin32Handle hEvent(::CreateEventW(nullptr, true, bInitSet, pwszName));
			if(!hEvent){
				DEBUG_THROW(SystemError, "CreateEventW");
			}
			return std::move(hEvent);
		}())
{
}
Event::Event(bool bInitSet, const WideString &wsName)
	: Event(bInitSet, wsName.GetStr())
{
}

// 其他非静态成员函数。
bool Event::Wait(unsigned long long ullMilliSeconds) const noexcept {
	const auto ullUntil = GetFastMonoClock() + ullMilliSeconds;
	bool bResult = false;
	auto ullTimeRemaining = ullMilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(xm_hEvent.Get(), Min(ullTimeRemaining, ULONG_MAX >> 1));
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
void Event::Wait() const noexcept {
	const auto dwResult = ::WaitForSingleObject(xm_hEvent.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
bool Event::IsSet() const noexcept {
	const auto dwResult = ::WaitForSingleObject(xm_hEvent.Get(), 0);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
	return dwResult != WAIT_TIMEOUT;
}
void Event::Set() noexcept {
	if(!::SetEvent(xm_hEvent.Get())){
		ASSERT_MSG(false, L"SetEvent() 失败。");
	}
}
void Event::Clear() noexcept {
	if(!::ResetEvent(xm_hEvent.Get())){
		ASSERT_MSG(false, L"ResetEvent() 失败。");
	}
}
