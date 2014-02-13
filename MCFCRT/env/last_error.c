// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "last_error.h"
#include <windows.h>

unsigned long __MCF_CRT_GetWin32LastError(void){
	return GetLastError();
}
void __MCF_CRT_SetWin32LastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}
