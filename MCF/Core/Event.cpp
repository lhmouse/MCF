// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

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
	UniqueHandle<xEventCloser> xm_hEvent;

public:
	xDelegate(bool bInitSet, const wchar_t *pwszName){
		xm_hEvent.Reset(::CreateEventW(nullptr, true, bInitSet, pwszName));
		if(!xm_hEvent){
			MCF_THROW(::GetLastError(), L"CreateEventW() 失败。");
		}
	}

public:
	HANDLE GetHandle() const noexcept {
		return xm_hEvent.Get();
	}
};

// 构造函数和析构函数。
Event::Event(bool bInitSet, const wchar_t *pwszName)
	: xm_pDelegate(new xDelegate(bInitSet, pwszName))
{
}
Event::Event(Event &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
Event &Event::operator=(Event &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
Event::~Event(){
}

// 其他非静态成员函数。
bool Event::IsSet() const noexcept {
	ASSERT(xm_pDelegate);

	return WaitTimeout(0);
}
void Event::Set() noexcept {
	ASSERT(xm_pDelegate);

	::SetEvent(xm_pDelegate->GetHandle());
}
void Event::Clear() noexcept {
	ASSERT(xm_pDelegate);

	::ResetEvent(xm_pDelegate->GetHandle());
}

bool Event::WaitTimeout(unsigned long ulMilliSeconds) const noexcept {
	ASSERT(xm_pDelegate);

	return ::WaitForSingleObject(xm_pDelegate->GetHandle(), ulMilliSeconds) != WAIT_TIMEOUT;
}
void Event::Wait() const noexcept {
	WaitTimeout(INFINITE);
}
