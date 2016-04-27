// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thread.hpp"
#include "../../MCFCRT/env/eh_top.h"
#include "../Core/Exception.hpp"

namespace MCF {

Thread::X_AbstractControlBlock::~X_AbstractControlBlock(){
}

void Thread::X_AbstractControlBlock::SpawnThread(bool bSuspended){
	MCF_ASSERT(!x_hThread);

	struct Helper {
		__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
		static DWORD ThreadProc(LPVOID pParam){
			const auto pThis = IntrusivePtr<X_AbstractControlBlock>(static_cast<X_AbstractControlBlock *>(pParam));

			__MCFCRT_EH_TOP_BEGIN
			{
				pThis->X_ThreadProc();
			}
			__MCFCRT_EH_TOP_END

			return 0;
		}
	};

	if(!x_hThread.Reset(::_MCFCRT_CreateNativeThread(&Helper::ThreadProc, this, true, &x_uTid))){
		MCF_THROW(Exception, ::GetLastError(), Rcntws::View(L"_MCFCRT_CreateThread() 失败。"));
	}
	AddRef();

	if(!bSuspended){
		::_MCFCRT_ResumeThread(x_hThread.Get());
	}
}

}
