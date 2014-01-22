// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Event.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
using namespace MCF;

// 嵌套类定义。
class Event::xDelegate : NO_COPY {
private:
	struct xEventCloser {
		constexpr HANDLE operator()() const {
			return NULL;
		}
		void operator()(HANDLE hEvent) const {
			::CloseHandle(hEvent);
		}
	};
private:
	UniqueHandle<HANDLE, xEventCloser> xm_hEvent;
public:
	xDelegate(bool bInitSet){
		UniqueHandle<HANDLE, xEventCloser> hEvent(::CreateEventW(nullptr, false, bInitSet, nullptr));
		if(!hEvent){
			MCF_THROW(::GetLastError(), L"CreateEventW() 失败。");
		}
		xm_hEvent = std::move(hEvent);
	}
public:
	HANDLE GetHandle() const noexcept {
		return xm_hEvent;
	}
};

// 构造函数和析构函数。
Event::Event(bool bInitSet)
	: xm_pDelegate(new xDelegate(bInitSet))
{
}
Event::~Event(){
}

// 其他非静态成员函数。
bool Event::IsSet() const noexcept {
	return WaitTimeOut(0);
}
void Event::Set() noexcept {
	::SetEvent(xm_pDelegate->GetHandle());
}
void Event::Reset() noexcept {
	::ResetEvent(xm_pDelegate->GetHandle());
}

bool Event::WaitTimeOut(unsigned long ulMilliSeconds) const noexcept {
	return ::WaitForSingleObject(xm_pDelegate->GetHandle(), ulMilliSeconds) != WAIT_TIMEOUT;
}
void Event::Wait() const noexcept {
	WaitTimeOut(INFINITE);
}
