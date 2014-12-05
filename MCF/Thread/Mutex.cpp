// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

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
		if(!xm_hMutex.Reset(::CreateMutexW(nullptr, false, pwszName))){
			DEBUG_THROW(SystemError, "CreateMutexW");
		}
	}

public:
	bool Try() const noexcept {
		const auto dwResult = ::WaitForSingleObject(xm_hMutex.Get(), 0);
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		return dwResult != WAIT_TIMEOUT;
	}
	void Wait() const noexcept {
		const auto dwResult = ::WaitForSingleObject(xm_hMutex.Get(), INFINITE);
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
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
	bool Mutex::Lock::xDoTry() const noexcept {
		ASSERT(dynamic_cast<MutexDelegate *>(xm_pOwner));

		return static_cast<MutexDelegate *>(xm_pOwner)->Try();
	}
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
std::unique_ptr<Mutex> Mutex::Create(const wchar_t *pwszName){
	return std::make_unique<MutexDelegate>(pwszName);
}
std::unique_ptr<Mutex> Mutex::Create(const WideString &wsName){
	return Create(wsName.GetCStr());
}

// 其他非静态成员函数。
Mutex::Lock Mutex::TryLock() noexcept {
	Lock vLock(this, 0);
	vLock.Try();
	return std::move(vLock);
}
Mutex::Lock Mutex::GetLock() noexcept {
	return Lock(this);
}
