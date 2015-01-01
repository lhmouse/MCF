// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include "mcfwin.h"

unsigned long MCF_CRT_GetWin32LastError(void){
	return GetLastError();
}
void MCF_CRT_SetWin32LastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}
