// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

namespace {
	union Control {
		std::uintptr_t u;

		__extension__ struct {
			std::size_t uIsLocked           : 1;
			std::size_t uWaitingThreads     : sizeof(std::uintptr_t) * CHAR_BIT - 1;
		};
	};

	static_assert(sizeof(Control) == sizeof(std::uintptr_t), "Fantastic struct layout?");
}

// 其他非静态成员函数。
bool Mutex::Try(std::uint64_t u64UntilUtcTime) noexcept {
	Control ctlOld, ctlNew;

	if(u64UntilUtcTime == 0){
		ctlOld.u = x_uControl.Load(kAtomicRelaxed);
	jCasFailureTest:
		ctlNew = ctlOld;
		ctlNew.uIsLocked = true;
		if(ctlOld.u != ctlNew.u){
			if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailureTest;
			}
			return true;
		}
		return false;
	}

	const auto u64HiResUtc = u64UntilUtcTime * 10000;
	const auto u64WaitUntil = u64HiResUtc + 0x019DB1DED53E8000ull;
	if((u64HiResUtc / 10000 != u64UntilUtcTime) || (u64WaitUntil >= 0x8000000000000000ull) || (u64WaitUntil < u64HiResUtc)){
		Lock();
		return true;
	}

	std::size_t uSpinnedCount = 0, uMaxSpinCount = x_uSpinCount.Load(kAtomicRelaxed);
	for(;;){
		ctlOld.u = x_uControl.Load(kAtomicRelaxed);
	jCasFailure:
		ctlNew = ctlOld;
		ctlNew.uIsLocked = true;
		if(ctlOld.u != ctlNew.u){
			if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			break;
		}

		if((ctlNew.uWaitingThreads == 0) && (uSpinnedCount < uMaxSpinCount)){
			AtomicPause();

			++uSpinnedCount;
		} else {
			++ctlNew.uWaitingThreads;
			if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			::LARGE_INTEGER liTimeout;
			liTimeout.QuadPart = static_cast<std::int64_t>(u64WaitUntil);
			const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, &liTimeout);
			if(!NT_SUCCESS(lStatus)){
				ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
			}
			if(lStatus == STATUS_TIMEOUT){
				ctlOld.u = x_uControl.Load(kAtomicRelaxed);
			jCasFailureTimedOut:
				ctlNew = ctlOld;
				if(ctlNew.uWaitingThreads == 0){
					if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
						goto jCasFailureTimedOut;
					}
					const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
					if(!NT_SUCCESS(lStatus)){
						ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
					}
				} else {
					--ctlNew.uWaitingThreads;
					if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
						goto jCasFailureTimedOut;
					}
				}
				return false;
			}

			uSpinnedCount = 0;
			uMaxSpinCount /= 2;
		}
	}
	return true;
}
void Mutex::Lock() noexcept {
	Control ctlOld, ctlNew;

	std::size_t uSpinnedCount = 0, uMaxSpinCount = x_uSpinCount.Load(kAtomicRelaxed);
	for(;;){
		ctlOld.u = x_uControl.Load(kAtomicRelaxed);
	jCasFailure:
		ctlNew = ctlOld;
		ctlNew.uIsLocked = true;
		if(ctlOld.u != ctlNew.u){
			if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			break;
		}

		if((ctlNew.uWaitingThreads == 0) && (uSpinnedCount < uMaxSpinCount)){
			AtomicPause();

			++uSpinnedCount;
		} else {
			++ctlNew.uWaitingThreads;
			if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
			if(!NT_SUCCESS(lStatus)){
				ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
			}

			uSpinnedCount = 0;
			uMaxSpinCount /= 2;
		}
	}
}
void Mutex::Unlock() noexcept {
	Control ctlOld, ctlNew;

	ctlOld.u = x_uControl.Load(kAtomicRelaxed);
jCasFailure:
	ctlNew = ctlOld;
	ASSERT_MSG(ctlNew.uIsLocked, L"互斥锁没有被任何线程锁定。");
	ctlNew.uIsLocked = false;
	if(ctlNew.uWaitingThreads == 0){
		if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
			goto jCasFailure;
		}
	} else {
		--ctlNew.uWaitingThreads;
		if(!x_uControl.CompareExchange(ctlOld.u, ctlNew.u, kAtomicSeqCst, kAtomicRelaxed)){
			goto jCasFailure;
		}
		const auto lStatus = ::NtReleaseKeyedEvent(nullptr, this, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
}

}
