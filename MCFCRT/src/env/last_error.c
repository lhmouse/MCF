// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "last_error.h"
#include "mcfwin.h"
#include <ntdef.h>
#include <winnt.h>

unsigned long _MCFCRT_GetLastWin32Error(void){
	return GetLastError();
}
void _MCFCRT_SetLastWin32Error(unsigned long ulErrorCode){
	SetLastError(ulErrorCode);
}

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS RtlFindMessage(void *pBaseAddress, DWORD dwUnknown, DWORD dwLanguageId, DWORD dwMessageId, MESSAGE_RESOURCE_ENTRY **ppEntry);

__attribute__((__dllimport__, __stdcall__))
extern void *RtlPcToFileHeader(void *pAddress, void **ppBase);

static inline bool IsLineBreak(wchar_t wc){
	return (wc == 0) || (wc == L'\n') || (wc == L'\r');
}

size_t _MCFCRT_GetWin32ErrorDescription(const wchar_t **ppwszStr, unsigned long ulErrorCode){
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
	const wchar_t *const pwcText = (void *)pEntry->Text;
	const wchar_t *pwcEnd = pwcText + (pEntry->Length - offsetof(MESSAGE_RESOURCE_ENTRY, Text)) / sizeof(wchar_t);
	for(;;){
		if(pwcText == pwcEnd){
			break;
		}
		if(!IsLineBreak(pwcEnd[-1])){
			break;
		}
		--pwcEnd;
	}
	const wchar_t *pwcBegin = pwcEnd;
	for(;;){
		if(pwcText == pwcBegin){
			break;
		}
		if(IsLineBreak(pwcBegin[-1])){
			break;
		}
		--pwcBegin;
	}
	*ppwszStr = pwcBegin;
	return (size_t)(pwcEnd - pwcBegin);
}
