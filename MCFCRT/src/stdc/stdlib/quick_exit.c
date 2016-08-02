// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/crt_module.h"

_Noreturn void quick_exit(int status){
	_MCFCRT_ExitProcess((unsigned)status, _MCFCRT_kExitTypeQuick);
}
