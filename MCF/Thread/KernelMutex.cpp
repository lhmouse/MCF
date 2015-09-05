// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "KernelMutex.hpp"
#include "WaitForSingleObject64.hpp"
#include "../Core/Exception.hpp"
#include "../Core/String.hpp"

namespace MCF {

namespace {
	UniqueWin32Handle CheckedCreateMutex(const wchar_t *pwszName){
		UniqueWin32Handle hMutex(::CreateMutexW(nullptr, false, pwszName));
		if(!hMutex){
			DEBUG_THROW(SystemError, "CreateMutexW");
		}
		return hMutex;
	}
}

template<>
bool KernelMutex::UniqueLock::$DoTry() const noexcept {
	return $pOwner->Try(0);
}
template<>
void KernelMutex::UniqueLock::$DoLock() const noexcept {
	$pOwner->Lock();
}
template<>
void KernelMutex::UniqueLock::$DoUnlock() const noexcept {
	$pOwner->Unlock();
}

// 构造函数和析构函数。
KernelMutex::KernelMutex(const wchar_t *pwszName)
	: $hMutex(CheckedCreateMutex(pwszName))
{
}
KernelMutex::KernelMutex(const WideString &wsName)
	: KernelMutex(wsName.GetStr())
{
}

// 其他非静态成员函数。
bool KernelMutex::Try(std::uint64_t u64MilliSeconds) noexcept {
	return WaitForSingleObject64($hMutex.Get(), &u64MilliSeconds);
}
void KernelMutex::Lock() noexcept {
	WaitForSingleObject64($hMutex.Get(), nullptr);
}
void KernelMutex::Unlock() noexcept {
	if(!::ReleaseMutex($hMutex.Get())){
		ASSERT_MSG(false, L"ReleaseMutex() 失败。");
	}
}

}
