// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include "standard_streams.h"
#include <ntdef.h>
#include <ntstatus.h>

typedef enum tagHardErrorResponseOption {
	kHardErrorAbortRetryIgnore,
	kHardErrorOk,
	kHardErrorOkCancel,
	kHardErrorRetryCancel,
	kHardErrorYesNo,
	kHardErrorYesNoCancel,
	kHardErrorShutdownSystem,
	kHardErrorTrayNotify,
	kHardErrorCancelTryAgainContinue,
} HardErrorResponseOption;

typedef enum tagHardErrorResponse {
	kHardErrorResponseReturnToCaller,
	kHardErrorResponseNotHandled,
	kHardErrorResponseAbort,
	kHardErrorResponseCancel,
	kHardErrorResponseIgnore,
	kHardErrorResponseNo,
	kHardErrorResponseOk,
	kHardErrorResponseRetry,
	kHardErrorResponseYes,
} HardErrorResponse;

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtRaiseHardError(NTSTATUS lStatus, DWORD dwParamCount, DWORD dwUnknown, const ULONG_PTR *pulParams, HardErrorResponseOption eOption, HardErrorResponse *peResponse);

_Noreturn void _MCFCRT_Bail(const wchar_t *pwszDescription){
	static volatile bool s_bBailing = false;

	const bool bBailing = __atomic_exchange_n(&s_bBailing, true, __ATOMIC_RELAXED);
	if(bBailing){
		TerminateThread(GetCurrentThread(), 3);
		__builtin_unreachable();
	}

#ifdef NDEBUG
	const bool bCanBeDebugged = IsDebuggerPresent();
#else
	const bool bCanBeDebugged = true;
#endif

	wchar_t awcBuffer[1024 + 128];
	wchar_t *pwcWrite = _MCFCRT_wcpcpy(awcBuffer, L"应用程序异常终止，请联系作者寻求协助。");
	if(pwszDescription){
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n错误描述：\n");
		pwcWrite = _MCFCRT_wcppcpy(pwcWrite, awcBuffer + 1024, pwszDescription); // 后面还有一些内容，保留一些字符。
	}
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n单击“确定”终止应用程序");
	if(bCanBeDebugged){
		pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"，单击“取消”调试应用程序");
	}
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"。");
	const size_t uLength = (size_t)(pwcWrite - awcBuffer);

	_MCFCRT_WriteStandardErrorText(awcBuffer, uLength, true);

	size_t uTextSizeInBytes = uLength * sizeof(wchar_t);
	const unsigned kMaxSizeInBytes = USHRT_MAX & -sizeof(wchar_t);
	if(uTextSizeInBytes > kMaxSizeInBytes){
		uTextSizeInBytes = kMaxSizeInBytes;
	}
	UNICODE_STRING ustrText;
	ustrText.Length        = (unsigned short)uTextSizeInBytes;
	ustrText.MaximumLength = ustrText.Length;
	ustrText.Buffer        = awcBuffer;

	static const wchar_t kCaption[] = L"MCF CRT";
	UNICODE_STRING ustrCaption;
	ustrCaption.Length        = sizeof(kCaption) - sizeof(wchar_t);
	ustrCaption.MaximumLength = ustrCaption.Length;
	ustrCaption.Buffer        = (wchar_t *)kCaption;

	const ULONG_PTR aulParams[] = {
		(ULONG_PTR)&ustrText,
		(ULONG_PTR)&ustrCaption,
		(ULONG_PTR)((bCanBeDebugged ? MB_OKCANCEL : MB_OK) | MB_ICONERROR),
		(ULONG_PTR)-1
	};
	HardErrorResponse eResponse;
	const NTSTATUS lStatus = NtRaiseHardError(0x50000018, sizeof(aulParams) / sizeof(aulParams[0]), 3, aulParams, kHardErrorOk, &eResponse);
	if(!NT_SUCCESS(lStatus)){
		eResponse = kHardErrorResponseCancel;
	}
	if(eResponse != kHardErrorResponseOk){
		DebugBreak();
	}
	TerminateProcess(GetCurrentProcess(), 3);
	__builtin_unreachable();
}
