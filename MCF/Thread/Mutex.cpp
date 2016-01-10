// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Mutex.hpp"
#include "../Core/Clocks.hpp"
#include <winternl.h>
#include <ntstatus.h>

extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;
extern "C" __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout) noexcept;

namespace MCF {

// 其他非静态成员函数。
bool Mutex::Try(std::uint64_t u64UntilFastMonoClock) noexcept {
	if(u64UntilFastMonoClock == 0){
		auto uOld = x_uControl.Load(kAtomicRelaxed);
	jCasFailureTest:
		auto uNew = uOld | 1;
		if(uNew != uOld){
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailureTest;
			}
			return true;
		}
		return false;
	}

	std::size_t uSpinnedCount = 0, uMaxSpinCount = x_uSpinCount.Load(kAtomicRelaxed);
	for(;;){
		auto uOld = x_uControl.Load(kAtomicRelaxed);
	jCasFailure:
		auto uNew = uOld | 1;
		if(uNew != uOld){
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			break;
		}

		if((uNew >> 1 == 0) && (uSpinnedCount < uMaxSpinCount)){
			AtomicPause();

			++uSpinnedCount;
		} else {
			uNew += 2;
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			::LARGE_INTEGER liTimeout;
			const auto u64Now = GetFastMonoClock();
			if(u64Now >= u64UntilFastMonoClock){
				liTimeout.QuadPart = 0;
			} else {
				const auto u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
				const auto n64Delta100Nanosec = static_cast<std::int64_t>(u64DeltaMillisec * 10000);
				if(static_cast<std::uint64_t>(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
					liTimeout.QuadPart = INT64_MIN;
				} else {
					liTimeout.QuadPart = -n64Delta100Nanosec;
				}
			}
			const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, &liTimeout);
			if(!NT_SUCCESS(lStatus)){
				ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
			}
			if(lStatus == STATUS_TIMEOUT){
				auto uOld = x_uControl.Load(kAtomicRelaxed);
			jCasFailureTimedOut:
				auto uNew = uOld;
				if(uNew >> 1 == 0){
					const auto lStatus = ::NtWaitForKeyedEvent(nullptr, this, false, nullptr);
					if(!NT_SUCCESS(lStatus)){
						ASSERT_MSG(false, L"NtWaitForKeyedEvent() 失败。");
					}
				} else {
					uNew -= 2;
					if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
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
	std::size_t uSpinnedCount = 0, uMaxSpinCount = x_uSpinCount.Load(kAtomicRelaxed);
	for(;;){
		auto uOld = x_uControl.Load(kAtomicRelaxed);
	jCasFailure:
		auto uNew = uOld | 1;
		if(uNew != uOld){
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
				goto jCasFailure;
			}
			break;
		}

		if((uNew >> 1 == 0) && (uSpinnedCount < uMaxSpinCount)){
			AtomicPause();

			++uSpinnedCount;
		} else {
			uNew += 2;
			if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
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
	auto uOld = x_uControl.Load(kAtomicRelaxed);
jCasFailure:
	ASSERT_MSG(uOld & 1, L"互斥锁没有被任何线程锁定。");
	auto uNew = (uOld >> 1) << 1;
	if(uNew == 0){
		if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
			goto jCasFailure;
		}
	} else {
		uNew -= 2;
		if(!x_uControl.CompareExchange(uOld, uNew, kAtomicSeqCst, kAtomicRelaxed)){
			goto jCasFailure;
		}
		const auto lStatus = ::NtReleaseKeyedEvent(nullptr, this, false, nullptr);
		if(!NT_SUCCESS(lStatus)){
			ASSERT_MSG(false, L"NtReleaseKeyedEvent() 失败。");
		}
	}
}

}
