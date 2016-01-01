// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include "mcfwin.h"
#include <winternl.h>
#include <ntdef.h>
#include <winnt.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS RtlFindMessage(void *pBaseAddress, DWORD dwUnknown, DWORD dwLanguageId, DWORD dwMessageId, MESSAGE_RESOURCE_ENTRY **ppEntry);

extern __attribute__((__dllimport__, __stdcall__))
void *RtlPcToFileHeader(void *pAddress, void **ppBase);

unsigned long MCF_CRT_GetWin32LastError(){
	return GetLastError();
}
void MCF_CRT_SetWin32LastError(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}

size_t MCF_CRT_GetWin32ErrorDescription(const wchar_t **ppwszStr, unsigned long ulErrorCode){
	static const wchar_t kUnknownErrorCode[]   = L"<未知错误码>";
	static const wchar_t kUnicodeUnavailable[] = L"<Unicode 错误码描述不可用>";

	void *pBaseAddress;
	RtlPcToFileHeader((void *)(intptr_t)&GetLastError, &pBaseAddress); // 获得 kernel32.dll 的基地址。

	MESSAGE_RESOURCE_ENTRY *pEntry;
	NTSTATUS lStatus = RtlFindMessage(pBaseAddress, 0x0B, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), ulErrorCode, &pEntry);
	if(!NT_SUCCESS(lStatus)){
		*ppwszStr = kUnknownErrorCode;
		return sizeof(kUnknownErrorCode) / sizeof(wchar_t) - 1;
	}
	if(pEntry->Flags != 0x0001){
		*ppwszStr = kUnicodeUnavailable;
		return sizeof(kUnicodeUnavailable) / sizeof(wchar_t) - 1;
	}
	const wchar_t *const pwcBegin = (const wchar_t *)pEntry->Text, *pwcEnd = pwcBegin + (pEntry->Length - offsetof(MESSAGE_RESOURCE_ENTRY, Text)) / sizeof(wchar_t);
	for(;;){
		if(pwcBegin == pwcEnd){
			break;
		}
		const wchar_t wcLast = pwcEnd[-1];
		if((wcLast != 0) && (wcLast != L'\n') && (wcLast != L'\r') && (wcLast != L' ') && (wcLast != L'\t')){
			break;
		}
		--pwcEnd;
	}
	*ppwszStr = pwcBegin;
	return (size_t)(pwcEnd - pwcBegin);
}
