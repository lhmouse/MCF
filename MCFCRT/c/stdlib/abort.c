// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include <windows.h>

__attribute__((noreturn, used)) void abort(){
	__MCF_Bail(L"应用程序调用 abort() 异常终止。");

	TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
	for(;;);
}
