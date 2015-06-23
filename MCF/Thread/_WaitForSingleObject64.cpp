// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "_WaitForSingleObject64.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

bool WaitForSingleObject64(void *hObject, unsigned long long *pullMilliSeconds) noexcept {
	if(pullMilliSeconds){
		const auto ullUntil = GetFastMonoClock() + *pullMilliSeconds;
		for(;;){
			const auto dwResult = ::WaitForSingleObject(hObject, Min(*pullMilliSeconds, 0x7FFFFFFFu));
			if(dwResult == WAIT_FAILED){
				ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
			}
			if(dwResult != WAIT_TIMEOUT){
				return true;
			}
			const auto ullNow = GetFastMonoClock();
			if(ullUntil <= ullNow){
				*pullMilliSeconds = 0;
				return false;
			}
			*pullMilliSeconds = ullUntil - ullNow;
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
