// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "Exception.hpp"
#include "UniqueHandle.hpp"
using namespace MCF;

namespace {

class MutexDelegate : CONCRETE(Mutex) {
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
	explicit MutexDelegate(const WideStringObserver &wsoName){
		const std::size_t uPathLen = wsoName.GetLength();
		if(uPathLen != 0){
			Vla<wchar_t> achNameZ(uPathLen + 1);
			std::copy(wsoName.GetBegin(), wsoName.GetEnd(), achNameZ.GetData());
			achNameZ[uPathLen] = 0;

			xm_hMutex.Reset(::CreateMutexW(nullptr, false, achNameZ.GetData()));
		} else {
			xm_hMutex.Reset(::CreateMutexW(nullptr, false, nullptr));
		}
		if(!xm_hMutex){
			MCF_THROW(::GetLastError(), L"CreateMutexW() 失败。");
		}
	}

public:
	bool WaitTimeout(unsigned long ulMilliSeconds) const noexcept {
		return ::WaitForSingleObject(xm_hMutex.Get(), ulMilliSeconds) != WAIT_TIMEOUT;
	}
	void Release() noexcept {
		if(!::ReleaseMutex(xm_hMutex.Get())){
			ASSERT_MSG(false, L"ReleaseMutex() 失败。");
		}
	}
};

}

// 静态成员函数。
std::unique_ptr<Mutex> Mutex::Create(const WideStringObserver &wsoName){
	return std::unique_ptr<Mutex>(new MutexDelegate(wsoName));
}

// 其他非静态成员函数。
bool Mutex::Try(unsigned long ulMilliSeconds) noexcept {
	ASSERT(dynamic_cast<MutexDelegate *>(this));

	return ((MutexDelegate *)this)->WaitTimeout(ulMilliSeconds);
}
void Mutex::Lock() noexcept {
	Try(INFINITE);
}
void Mutex::Unlock() noexcept {
	ASSERT(dynamic_cast<MutexDelegate *>(this));

	((MutexDelegate *)this)->Release();
}
