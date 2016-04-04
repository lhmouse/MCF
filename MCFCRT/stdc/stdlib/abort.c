// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include "../../env/last_error.h"
#include <setjmp.h>

jmp_buf *__MCFCRT_pjbufAbortHook = nullptr;

_Noreturn void abort(){
	if(__MCFCRT_pjbufAbortHook){
		longjmp(*__MCFCRT_pjbufAbortHook, ERROR_PROCESS_ABORTED);
	}

	_MCFCRT_Bail(L"应用程序调用了 abort()。");
}
