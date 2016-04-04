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

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtRaiseHardError(NTSTATUS stError, DWORD dwUnknown, DWORD dwParamCount, const ULONG_PTR *pulParams, HardErrorResponseOption eOption, HardErrorResponse *peResponse);

_Noreturn void _MCFCRT_Bail(const wchar_t *pwszDescription){
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
		pwcWrite = _MCFCRT_wcppcpy(pwcWrite, awcBuffer + 1024 + 128, pwszDescription); // 后面还有一些内容，保留一些字符。
	}
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n单击“确定”终止应用程序");
	if(bCanBeDebugged){
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"，单击“取消”调试应用程序");
	}
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"。\n");

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
	ustrText.Length        = (unsigned short)((char *)pwcWrite - (char *)awcBuffer);
	ustrText.MaximumLength = ustrText.Length;
	ustrText.Buffer        = awcBuffer;

	static const wchar_t kCaption[] = L"MCF CRT 错误";
	UNICODE_STRING ustrCaption;
	ustrCaption.Length        = sizeof(kCaption) - sizeof(wchar_t);
	ustrCaption.MaximumLength = ustrCaption.Length;
	ustrCaption.Buffer        = (wchar_t *)kCaption;

	UINT uType = (bCanBeDebugged ? MB_OKCANCEL : MB_OK) | MB_ICONERROR;

	const ULONG_PTR aulParams[3] = { (ULONG_PTR)&ustrText, (ULONG_PTR)&ustrCaption, uType };
	HardErrorResponse eResponse;
	const NTSTATUS lStatus = NtRaiseHardError(0x50000018, 4, 3, aulParams, (bCanBeDebugged ? kHardErrorOkCancel : kHardErrorOk), &eResponse);
	if(NT_SUCCESS(lStatus)){
		bShouldGenerateBreakpoint = (eResponse != kHardErrorResponseOk);
	}

	if(bShouldGenerateBreakpoint){
		__asm__ volatile ("int3 \n");
	}
	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_UNSUCCESSFUL);
	__builtin_unreachable();
}
