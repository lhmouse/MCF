// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/System.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

// 其他非静态成员函数。
bool Mutex::Try(std::uint64_t u64MilliSeconds) noexcept {
	return ::TryAcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(&x_uControl));
}
void Mutex::Lock() noexcept {
	::AcquireSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(&x_uControl));
}
void Mutex::Unlock() noexcept {
	::ReleaseSRWLockExclusive(reinterpret_cast<::SRWLOCK *>(&x_uControl));
}

}
