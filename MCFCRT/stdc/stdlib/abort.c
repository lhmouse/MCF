// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/bail.h"
#include "../../env/last_error.h"

_Noreturn void __wrap_abort(){
	_MCFCRT_Bail(L"应用程序调用了 abort()。");
}

__attribute__((__alias__("__wrap_abort")))
_Noreturn void abort();
