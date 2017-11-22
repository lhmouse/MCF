// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include <windows.h>

unsigned long MCFBUILD_GetLastError(void) MCFBUILD_NOEXCEPT {
	return GetLastError();
}
void MCFBUILD_SetLastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}
