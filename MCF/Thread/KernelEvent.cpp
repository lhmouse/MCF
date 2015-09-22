// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelEvent.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace {
	UniqueWin32Handle CheckedCreateEvent(bool bInitSet, const wchar_t *pwszName){
		UniqueWin32Handle hEvent(::CreateEventW(nullptr, true, bInitSet, pwszName));
		if(!hEvent){
			DEBUG_THROW(SystemError, "CreateEventW"_rcs);
		}
		return hEvent;
	}
}

// 构造函数和析构函数。
KernelEvent::KernelEvent(bool bInitSet, const wchar_t *pwszName)
	: x_hEvent(CheckedCreateEvent(bInitSet, pwszName))
{
}
KernelEvent::KernelEvent(bool bInitSet, const WideString &wsName)
	: KernelEvent(bInitSet, wsName.GetStr())
{
}

// 其他非静态成员函数。
bool KernelEvent::Wait(std::uint64_t u64MilliSeconds) const noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hEvent.Get(), Min(u64Until - u64Now, 0x7FFFFFFFu));
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
void KernelEvent::Wait() const noexcept {
	const auto dwResult = ::WaitForSingleObject(x_hEvent.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
bool KernelEvent::IsSet() const noexcept {
	return Wait(0);
}
void KernelEvent::Set() noexcept {
	if(!::SetEvent(x_hEvent.Get())){
		ASSERT_MSG(false, L"SetEvent() 失败。");
	}
}
void KernelEvent::Reset() noexcept {
	if(!::ResetEvent(x_hEvent.Get())){
		ASSERT_MSG(false, L"ResetEvent() 失败。");
	}
}

}
