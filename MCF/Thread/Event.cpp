// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

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
		xm_hEvent.Reset(::CreateEventW(nullptr, true, bInitSet, pwszName));
		if(!xm_hEvent){
			MCF_THROW(::GetLastError(), L"CreateEventW() 失败。"_wso);
		}
	}

public:
	bool WaitTimeout(unsigned long long ullMilliSeconds) const noexcept {
		return WaitUntil(
			[&](DWORD dwRemaining) noexcept {
				return ::WaitForSingleObject(xm_hEvent.Get(), dwRemaining) != WAIT_TIMEOUT;
			},
			ullMilliSeconds
		);
	}

	void Set() const noexcept {
		::SetEvent(xm_hEvent.Get());
	}
	void Clear() const noexcept {
		::ResetEvent(xm_hEvent.Get());
	}
};

}

// 静态成员函数。
std::unique_ptr<Event> Event::Create(bool bInitSet, const WideStringObserver &wsoName){
	return std::make_unique<EventDelegate>(bInitSet, wsoName.IsEmpty() ? nullptr : wsoName.GetNullTerminated<MAX_PATH>().GetData());
}
std::unique_ptr<Event> Event::Create(bool bInitSet, const WideString &wcsName){
	return std::make_unique<EventDelegate>(bInitSet, wcsName.GetCStr());
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
