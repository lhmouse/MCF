// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "Thread.hpp"
#include <MCFCRT/env/_seh_top.h>
#include "../Core/Exception.hpp"

namespace MCF {

Thread::~Thread(){
}

void Thread::X_Spawn(bool bSuspended){
	struct Helper {
		__MCFCRT_C_STDCALL
		static unsigned long NativeThreadProc(void *pParam){
			const auto pThis = IntrusivePtr<const Thread>(static_cast<const Thread *>(pParam));

			__MCFCRT_SEH_TOP_BEGIN
			{
				pThis->X_ThreadProc();
			}
			__MCFCRT_SEH_TOP_END

			return 0;
		}
	};
	MCF_ASSERT(!x_hThread);
	AddRef();
	if(!x_hThread.Reset(::_MCFCRT_CreateNativeThread(&Helper::NativeThreadProc, this, bSuspended, &x_uThreadId))){
		DropRef();
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"_MCFCRT_CreateThread() 失败。"));
	}
}

}
