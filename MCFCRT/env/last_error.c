// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include "mcfwin.h"

unsigned long MCF_CRT_GetWin32LastError(void){
	return GetLastError();
}
void MCF_CRT_SetWin32LastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}

size_t MCF_CRT_GetWin32ErrorDesc(
	wchar_t *pwchBuffer, size_t uBufferSize,
	unsigned long ulErrorCode
){
	return FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, ulErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		pwchBuffer, uBufferSize,
		NULL
	);
}
