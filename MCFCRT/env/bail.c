// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include <stdarg.h>
#include <wchar.h>

#include <ntdef.h>
#include <ntstatus.h>

// ntdll.dll
extern
__attribute__((__dllimport__, __stdcall__))
VOID RtlInitUnicodeString(PUNICODE_STRING pDestination, PCWSTR pwszSource);

typedef enum tagHardErrorResponseOption {
	kHardErrorAbortRetryIgnore,
	kHardErrorOk,
	kHardErrorOkCancel,
	kHardErrorRetryCancel,
	kHardErrorYesNo,
	kHardErrorYesNoCancel,
	kHardErrorShutdownSystem,
} HardErrorResponseOption;

typedef enum tagHardErrorResponse {
	kHardErrorResponseUnknown0,
	kHardErrorResponseUnknown1,
	kHardErrorResponseAbort,
	kHardErrorResponseCancel,
	kHardErrorResponseIgnore,
	kHardErrorResponseNo,
	kHardErrorResponseOk,
	kHardErrorResponseRetry,
	kHardErrorResponseYes,
} HardErrorResponse;

extern
__attribute__((__dllimport__, __stdcall__))
NTSTATUS NtRaiseHardError(NTSTATUS stError, DWORD dwUnknown, DWORD dwParamCount, const ULONG_PTR *pulParams, HardErrorResponseOption eOption, HardErrorResponse *peResponse);

__attribute__((__noreturn__))
static void DoBail(const wchar_t *pwszDescription){
#ifdef NDEBUG
	const bool bCanBeDebugged = IsDebuggerPresent();
#else
	const bool bCanBeDebugged = true;
#endif

	wchar_t awcBuffer[1024 + 256];
	wchar_t *pwcWrite = MCF_wcpcpy(awcBuffer, L"应用程序异常终止，请联系作者寻求协助。");
	if(pwszDescription){
		pwcWrite = MCF_wcpcpy(pwcWrite, L"\n\n错误描述：\n");

		size_t uLen = wcslen(pwszDescription);
		const size_t uMax = (size_t)(awcBuffer + sizeof(awcBuffer) / sizeof(wchar_t) - pwcWrite) - 64; // 后面还有一些内容，保留一些字符。
		if(uLen > uMax){
			uLen = uMax;
		}
		wmemcpy(pwcWrite, pwszDescription, uLen);
		pwcWrite += uLen;
	}
	pwcWrite = MCF_wcpcpy(pwcWrite,
		bCanBeDebugged ? L"\n\n单击“确定”终止应用程序，单击“取消”调试应用程序。\n" : L"\n\n单击“确定”终止应用程序。\n");

	const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	if(hStdErr != INVALID_HANDLE_VALUE){
		DWORD dwMode;
		if(GetConsoleMode(hStdErr, &dwMode)){
			const wchar_t *pwcRead = awcBuffer;
			for(;;){
				const DWORD dwCharsToWrite = (DWORD)(pwcWrite - pwcRead);
				if(dwCharsToWrite == 0){
					break;
				}
				DWORD dwCharsWritten;
				if(!WriteConsoleW(hStdErr, pwcRead, dwCharsToWrite, &dwCharsWritten, nullptr)){
					break;
				}
				pwcRead += dwCharsWritten;
			}
		}
	}
	*(pwcWrite--) = 0;

	UNICODE_STRING ustrText;
	UNICODE_STRING ustrCaption;
	UINT uType;

	RtlInitUnicodeString(&ustrText, awcBuffer);
	RtlInitUnicodeString(&ustrCaption, L"MCF CRT 错误");
	uType = (bCanBeDebugged ? MB_OKCANCEL : MB_OK) | MB_ICONERROR;

	const ULONG_PTR aulParams[3] = { (ULONG_PTR)&ustrText, (ULONG_PTR)&ustrCaption, uType };
	HardErrorResponse eResponse;
	if(!NT_SUCCESS(NtRaiseHardError(STATUS_SERVICE_NOTIFICATION, 4, 3, aulParams, (bCanBeDebugged ? kHardErrorOkCancel : kHardErrorOk), &eResponse))){
		eResponse = kHardErrorResponseCancel;
	}
	if(bCanBeDebugged && (eResponse != kHardErrorResponseOk)){
		__asm__ __volatile__("int3 \n");
	}

	TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
	__builtin_unreachable();
}

__attribute__((__noreturn__))
void MCF_CRT_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}
__attribute__((__noreturn__))
void MCF_CRT_BailF(const wchar_t *pwszFormat, ...){
	wchar_t awcBuffer[1024];
	va_list ap;
	va_start(ap, pwszFormat);
	vswprintf(awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t), pwszFormat, ap);
	va_end(ap);
	DoBail(awcBuffer);
}
