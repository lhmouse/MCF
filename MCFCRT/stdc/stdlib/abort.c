// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include "../../env/mcfwin.h"

_Noreturn void abort(){
	MCF_CRT_Bail(L"应用程序调用了 abort()。");

	TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
	__builtin_unreachable();
}
