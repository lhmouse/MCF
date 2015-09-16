// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include "../../env/mcfwin.h"
#include <setjmp.h>

jmp_buf *__MCF_CRT_abort_hool_jmpbuf = nullptr;

_Noreturn void abort(){
	if(__MCF_CRT_abort_hool_jmpbuf){
		longjmp(*__MCF_CRT_abort_hool_jmpbuf, ERROR_PROCESS_ABORTED);
	}

	MCF_CRT_Bail(L"应用程序调用了 abort()。");
	TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
	__builtin_unreachable();
}
