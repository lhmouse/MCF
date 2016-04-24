// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include <ntdef.h>
#include <ntstatus.h>

static volatile bool g_bBailed = false;

_Noreturn void _MCFCRT_Bail(const wchar_t *pwszDescription){
	const bool bBailed = __atomic_exchange_n(&g_bBailed, true, __ATOMIC_RELAXED);
	if(bBailed){
		TerminateThread(GetCurrentThread(), (DWORD)STATUS_UNSUCCESSFUL);
		__builtin_unreachable();
	}

#ifdef NDEBUG
	const bool bCanBeDebugged = IsDebuggerPresent();
#else
	const bool bCanBeDebugged = true;
#endif

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

	const int nResponse = MessageBoxW(nullptr, awcBuffer, L"MCF CRT 错误", (bCanBeDebugged ? MB_OKCANCEL : MB_OK) | MB_ICONERROR | MB_SERVICE_NOTIFICATION);
	if(nResponse != IDOK){
		__debugbreak();
	}
	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_UNSUCCESSFUL);
	__builtin_unreachable();
}
