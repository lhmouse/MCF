// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
using namespace MCF;

// 嵌套类定义。
class Mutex::xDelegate : NO_COPY {
private:
	struct xMutexCloser {
		constexpr HANDLE operator()() const {
			return NULL;
		}
		void operator()(HANDLE hMutex) const {
			::CloseHandle(hMutex);
		}
	};

private:
	UniqueHandle<xMutexCloser> xm_hMutex;

public:
	xDelegate(const wchar_t *pwszName){
		xm_hMutex.Reset(::CreateMutexW(nullptr, FALSE, pwszName));
		if(!xm_hMutex){
			MCF_THROW(::GetLastError(), L"CreateMutexW() 失败。");
		}
	}

public:
	HANDLE GetHandle() const noexcept {
		return xm_hMutex.Get();
	}
};

// 静态成员函数。
void Mutex::xUnlock(Mutex::xDelegate *pDelegate) noexcept {
	if(pDelegate){
		::ReleaseMutex(pDelegate->GetHandle());
	}
}

// 构造函数和析构函数。
Mutex::Mutex(const wchar_t *pwszName)
	: xm_pDelegate(new xDelegate(pwszName))
{
}
Mutex::Mutex(Mutex &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
Mutex &Mutex::operator=(Mutex &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
Mutex::~Mutex(){
}

// 其他非静态成员函数。
Mutex::LockHolder Mutex::Try() noexcept {
	ASSERT(xm_pDelegate);

	if(::WaitForSingleObject(xm_pDelegate->GetHandle(), 0) == WAIT_TIMEOUT){
		return LockHolder();
	}
	return LockHolder(xm_pDelegate.get());
}
Mutex::LockHolder Mutex::Lock() noexcept {
	ASSERT(xm_pDelegate);

	::WaitForSingleObject(xm_pDelegate->GetHandle(), INFINITE);
	return LockHolder(xm_pDelegate.get());
}
