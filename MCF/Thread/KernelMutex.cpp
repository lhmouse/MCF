// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelMutex.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

namespace {
	UniqueWin32Handle CheckedCreateMutex(const wchar_t *pwszName){
		UniqueWin32Handle hMutex(::CreateMutexW(nullptr, false, pwszName));
		if(!hMutex){
			DEBUG_THROW(SystemError, "CreateMutexW"_rcs);
		}
		return hMutex;
	}
}

namespace Impl_UniqueLockTemplate {
	template<>
	bool KernelMutex::UniqueLock::X_DoTry() const noexcept {
		return x_pOwner->Try(0);
	}
	template<>
	void KernelMutex::UniqueLock::X_DoLock() const noexcept {
		x_pOwner->Lock();
	}
	template<>
	void KernelMutex::UniqueLock::X_DoUnlock() const noexcept {
		x_pOwner->Unlock();
	}
}

// 构造函数和析构函数。
KernelMutex::KernelMutex(const wchar_t *pwszName)
	: x_hMutex(CheckedCreateMutex(pwszName))
{
}
KernelMutex::KernelMutex(const WideString &wsName)
	: KernelMutex(wsName.GetStr())
{
}

// 其他非静态成员函数。
bool KernelMutex::Try(std::uint64_t u64MilliSeconds) noexcept {
	auto u64Now = GetFastMonoClock();
	const auto u64Until = u64Now + u64MilliSeconds;
	for(;;){
		const auto dwResult = ::WaitForSingleObject(x_hMutex.Get(), Min(u64Until - u64Now, 0x7FFFFFFFu));
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
void KernelMutex::Lock() noexcept {
	const auto dwResult = ::WaitForSingleObject(x_hMutex.Get(), INFINITE);
	if(dwResult == WAIT_FAILED){
		ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
	}
}
void KernelMutex::Unlock() noexcept {
	if(!::ReleaseMutex(x_hMutex.Get())){
		ASSERT_MSG(false, L"ReleaseMutex() 失败。");
	}
}

}
