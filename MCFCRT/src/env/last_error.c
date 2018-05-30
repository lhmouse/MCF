// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include "mcfwin.h"
#include <ntdef.h>
#include <winnt.h>

unsigned long _MCFCRT_GetLastError(void){
	return GetLastError();
}
void _MCFCRT_SetLastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}

__attribute__((__dllimport__, __stdcall__)) extern NTSTATUS RtlFindMessage(void *pBaseAddress, DWORD dwUnknown, DWORD dwLanguageId, DWORD dwMessageId, MESSAGE_RESOURCE_ENTRY **ppEntry);
__attribute__((__dllimport__, __stdcall__)) extern void * RtlPcToFileHeader(void *pAddress, void **ppBase);

static inline bool IsLineBreak(wchar_t wcChar){
	return (wcChar == 0) || (wcChar == L'\n') || (wcChar == L'\r');
}

bool _MCFCRT_GetErrorDescription(const wchar_t **restrict ppwszText, size_t *restrict puLength, unsigned long ulErrorCode){
	void *pBaseAddress;
	RtlPcToFileHeader((void *)(intptr_t)&GetLastError, &pBaseAddress); // 获得 kernel32.dll 的基地址。
	MESSAGE_RESOURCE_ENTRY *pEntry;
	NTSTATUS lStatus = RtlFindMessage(pBaseAddress, 0x0B, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), ulErrorCode, &pEntry);
	if(!NT_SUCCESS(lStatus)){
		static const wchar_t kUnknownErrorCode[]   = L"<未知错误码>";
		*ppwszText = kUnknownErrorCode;
		*puLength = sizeof(kUnknownErrorCode) / sizeof(wchar_t) - 1;
		return false;
	}
	if(pEntry->Flags != 0x0001){
		static const wchar_t kUnicodeUnavailable[] = L"<Unicode 错误码描述不可用>";
		*ppwszText = kUnicodeUnavailable;
		*puLength = sizeof(kUnicodeUnavailable) / sizeof(wchar_t) - 1;
		return false;
	}
	const wchar_t *const pwcBegin = (const void *)pEntry->Text;
	// Strip trailing line break characters.
	const wchar_t *pwcLineEnd = (const void *)((const unsigned char *)pEntry + pEntry->Length);
	while((pwcBegin < pwcLineEnd) && IsLineBreak(pwcLineEnd[-1])){
		--pwcLineEnd;
	}
	// Strip everything before the last line.
	const wchar_t *pwcLineBegin = pwcLineEnd;
	while((pwcBegin < pwcLineBegin) && !IsLineBreak(pwcLineBegin[-1])){
		--pwcLineBegin;
	}
	*ppwszText = pwcBegin;
	*puLength = (size_t)(pwcLineEnd - pwcBegin);
	return true;
}
