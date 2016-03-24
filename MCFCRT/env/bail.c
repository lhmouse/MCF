// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include <stdarg.h>
#include <wchar.h>

#include <ntdef.h>
#include <ntstatus.h>
#include <winternl.h>

// ntdll.dll
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

_Noreturn
static void DoBail(const wchar_t *pwszDescription){
#ifdef NDEBUG
	const bool bCanBeDebugged = IsDebuggerPresent();
#else
	const bool bCanBeDebugged = true;
#endif
	bool bShouldGenerateBreakpoint = bCanBeDebugged;

	wchar_t awcBuffer[1024 + 256];
	wchar_t *pwcWrite = _MCFCRT_wcpcpy(awcBuffer, L"应用程序异常终止，请联系作者寻求协助。");
	if(pwszDescription){
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n错误描述：\n");
		wchar_t *const pwcEnd = awcBuffer + sizeof(awcBuffer) / sizeof(wchar_t) - 64; // 后面还有一些内容，保留一些字符。
		pwcWrite = _MCFCRT_wcppcpy(pwcWrite, pwcEnd, pwszDescription);
	}
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, bCanBeDebugged ? L"\n\n单击“确定”终止应用程序，单击“取消”调试应用程序。\n" : L"\n\n单击“确定”终止应用程序。\n");

	const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	if(hStdErr != INVALID_HANDLE_VALUE){
		DWORD dwMode;
		if(GetConsoleMode(hStdErr, &dwMode)){
			DWORD dwCharsWritten;
			WriteConsoleW(hStdErr, awcBuffer, (DWORD)(pwcWrite - awcBuffer), &dwCharsWritten, nullptr);
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
	const NTSTATUS lStatus = NtRaiseHardError(STATUS_SERVICE_NOTIFICATION, 4, 3, aulParams, (bCanBeDebugged ? kHardErrorOkCancel : kHardErrorOk), &eResponse);
	if(NT_SUCCESS(lStatus)){
		bShouldGenerateBreakpoint = (eResponse != kHardErrorResponseOk);
	}

	if(bShouldGenerateBreakpoint){
		__asm__ __volatile__("int3 \n");
	}
	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_UNSUCCESSFUL);
	__builtin_unreachable();
}

_Noreturn
void _MCFCRT_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}

_Noreturn
void _MCFCRT_BailV(const wchar_t *pwszFormat, va_list pArgs){
	wchar_t awcBuffer[1024];
	vswprintf(awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t), pwszFormat, pArgs);
	DoBail(awcBuffer);
}
