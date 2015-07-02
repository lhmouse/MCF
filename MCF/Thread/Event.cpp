// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"
#include "WaitForSingleObject64.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"

namespace MCF {

namespace {
	UniqueWin32Handle CheckedCreateEvent(bool bInitSet, const wchar_t *pwszName){
		UniqueWin32Handle hEvent(::CreateEventW(nullptr, true, bInitSet, pwszName));
		if(!hEvent){
			DEBUG_THROW(SystemError, "CreateEventW");
		}
		return hEvent;
	}
}

// 构造函数和析构函数。
Event::Event(bool bInitSet, const wchar_t *pwszName)
	: x_hEvent(CheckedCreateEvent(bInitSet, pwszName))
{
}
Event::Event(bool bInitSet, const WideString &wsName)
	: Event(bInitSet, wsName.GetStr())
{
}

// 其他非静态成员函数。
bool Event::Wait(unsigned long long ullMilliSeconds) const noexcept {
	return WaitForSingleObject64(x_hEvent.Get(), &ullMilliSeconds);
}
void Event::Wait() const noexcept {
	WaitForSingleObject64(x_hEvent.Get(), nullptr);
}
bool Event::IsSet() const noexcept {
	const auto dwResult = ::WaitForSingleObject(x_hEvent.Get(), 0);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
	return dwResult != WAIT_TIMEOUT;
}
void Event::Set() noexcept {
	if(!::SetEvent(x_hEvent.Get())){
		ASSERT_MSG(false, L"SetEvent() 失败。");
	}
}
void Event::Clear() noexcept {
	if(!::ResetEvent(x_hEvent.Get())){
		ASSERT_MSG(false, L"ResetEvent() 失败。");
	}
}

}
