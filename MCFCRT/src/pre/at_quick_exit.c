// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "../env/crt_module.h"

int at_quick_exit(void (*func)(void)){
	return _MCFCRT_AtCrtModuleQuickExit((_MCFCRT_AtCrtModuleExitCallback)func, 0) ? 0 : -1;
}
