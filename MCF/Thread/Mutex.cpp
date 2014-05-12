// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/UniqueHandle.hpp"
using namespace MCF;

namespace {

class MutexDelegate : CONCRETE(Mutex) {
private:
	struct xMutexCloser {
		constexpr HANDLE operator()() const noexcept {
			return NULL;
		}
		void operator()(HANDLE hMutex) const noexcept {
			::CloseHandle(hMutex);
		}
	};

private:
	UniqueHandle<xMutexCloser> xm_hMutex;

public:
	explicit MutexDelegate(const WideStringObserver &wsoName){
		if(wsoName.IsEmpty()){
			xm_hMutex.Reset(::CreateMutexW(nullptr, false, nullptr));
		} else {
			xm_hMutex.Reset(::CreateMutexW(nullptr, false, wsoName.GetNullTerminated<MAX_PATH>().GetData()));
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
	return std::make_unique<MutexDelegate>(wsoName);
}

// 其他非静态成员函数。
void Mutex::Lock() noexcept {
	ASSERT(dynamic_cast<MutexDelegate *>(this));

	((MutexDelegate *)this)->WaitTimeout(INFINITE);
}
void Mutex::Unlock() noexcept {
	ASSERT(dynamic_cast<MutexDelegate *>(this));

	((MutexDelegate *)this)->Release();
}
