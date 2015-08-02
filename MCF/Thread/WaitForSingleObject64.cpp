// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "WaitForSingleObject64.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

bool WaitForSingleObject64(void *hObject, std::uint64_t *pu64MilliSeconds) noexcept {
	if(pu64MilliSeconds){
		const auto u64Until = GetFastMonoClock() + *pu64MilliSeconds;
		for(;;){
			const auto dwResult = ::WaitForSingleObject(hObject, Min(*pu64MilliSeconds, 0x7FFFFFFFu));
			if(dwResult == WAIT_FAILED){
				ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
			}
			const auto u64Now = GetFastMonoClock();
			if(u64Until <= u64Now){
				*pu64MilliSeconds = 0;
			} else {
				*pu64MilliSeconds = u64Until - u64Now;
			}

			if(dwResult != WAIT_TIMEOUT){
				return true;
			}
			if(u64Until <= u64Now){
				return false;
			}
		}
	} else {
		const auto dwResult = ::WaitForSingleObject(hObject, INFINITE);
		if(dwResult == WAIT_FAILED){
			ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
		}
		return true;
	}
}

}
