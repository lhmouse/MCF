// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

class EventDelegate : CONCRETE(Event) {
private:
	Impl::UniqueWinHandle xm_hEvent;

public:
	EventDelegate(bool bInitSet, const wchar_t *pwszName){
		if(!xm_hEvent.Reset(::CreateEventW(nullptr, true, bInitSet, pwszName))){
			DEBUG_THROW(SystemError, "CreateEventW");
		}
	}

public:
	bool WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
		return WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				const auto dwResult = ::WaitForSingleObject(xm_hEvent.Get(), dwRemaining);
				if(dwResult == WAIT_FAILED){
					ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
				}
				return dwResult != WAIT_TIMEOUT;
			},
			ullMilliSeconds
		);
	}

	void Set() const noexcept {
		if(!::SetEvent(xm_hEvent.Get())){
			ASSERT_MSG(false, L"SetEvent() 失败。");
		}
	}
	void Clear() const noexcept {
		if(!::ResetEvent(xm_hEvent.Get())){
			ASSERT_MSG(false, L"ResetEvent() 失败。");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<Event> Event::Create(bool bInitSet, const wchar_t *pwszName){
	return std::make_unique<EventDelegate>(bInitSet, pwszName);
}
std::unique_ptr<Event> Event::Create(bool bInitSet, const WideString &wsName){
	return Create(bInitSet, wsName.GetCStr());
}

// 其他非静态成员函数。
bool Event::IsSet() const noexcept {
	return WaitTimeout(0);
}
void Event::Set() noexcept {
	ASSERT(dynamic_cast<EventDelegate *>(this));

	static_cast<EventDelegate *>(this)->Set();
}
void Event::Clear() noexcept {
	ASSERT(dynamic_cast<EventDelegate *>(this));

	static_cast<EventDelegate *>(this)->Clear();
}

bool Event::WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
	ASSERT(dynamic_cast<const EventDelegate *>(this));

	return ((const EventDelegate *)this)->WaitTimeout(ullMilliSeconds);
}
void Event::Wait() const noexcept {
	WaitTimeout(WAIT_FOREVER);
}
