// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
using namespace MCF;

namespace {

class MutexDelegate : CONCRETE(Mutex) {
private:
	Impl::UniqueWinHandle xm_hMutex;

public:
	explicit MutexDelegate(const wchar_t *pwszName){
		xm_hMutex.Reset(::CreateMutexW(nullptr, false, pwszName));
		if(!xm_hMutex){
			MCF_THROW(::GetLastError(), L"CreateMutexW() 失败。"_wso);
		}
	}

public:
	void Wait() const noexcept {
		::WaitForSingleObject(xm_hMutex.Get(), INFINITE);
	}
	void Release() const noexcept {
		if(!::ReleaseMutex(xm_hMutex.Get())){
			ASSERT_MSG(false, L"ReleaseMutex() 失败。");
		}
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	void Mutex::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<MutexDelegate *>(xm_pOwner));

		static_cast<MutexDelegate *>(xm_pOwner)->Wait();
	}
	template<>
	void Mutex::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<MutexDelegate *>(xm_pOwner));

		static_cast<MutexDelegate *>(xm_pOwner)->Release();
	}
}

}

// 静态成员函数。
std::unique_ptr<Mutex> Mutex::Create(const WideStringObserver &wsoName){
	return std::make_unique<MutexDelegate>(wsoName.IsEmpty() ? nullptr : wsoName.GetNullTerminated<MAX_PATH>().GetData());
}
std::unique_ptr<Mutex> Mutex::Create(const WideString &wcsName){
	return std::make_unique<MutexDelegate>(wcsName.GetCStr());
}

// 其他非静态成员函数。
Mutex::Lock Mutex::GetLock() noexcept {
	return Lock(this);
}
