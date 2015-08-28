// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/wcpcpy.h"
#include <stdarg.h>
#include <wchar.h>
#include <stdlib.h>

#define MAX_MESSAGE_BUFFER_SIZE		(1025 + 256)

static DWORD APIENTRY ThreadProc(LPVOID pParam){
	return (DWORD)MessageBoxW(nullptr, pParam, L"MCF CRT 错误", MB_ICONERROR | MB_TASKMODAL |
#ifndef NDEBUG
		MB_OKCANCEL
#else
		MB_OK
#endif
		);
}

MCF_CRT_NORETURN_IF_NDEBUG static void DoBail(const wchar_t *pwszDescription){
	wchar_t awcBuffer[MAX_MESSAGE_BUFFER_SIZE];
	wchar_t *pwcMsgEnd = MCF_wcpcpy(awcBuffer, L"应用程序异常终止。请联系作者寻求协助。");
	if(pwszDescription){
		pwcMsgEnd = MCF_wcpcpy(pwcMsgEnd, L"\r\n\r\n错误描述：\r\n");

		size_t uLen = wcslen(pwszDescription);
		const size_t uMax = (size_t)(awcBuffer + MAX_MESSAGE_BUFFER_SIZE - pwcMsgEnd) - 64; // 后面还有一些内容，保留一些字符。
		if(uLen > uMax){
			uLen = uMax;
		}
		wmemcpy(pwcMsgEnd, pwszDescription, uLen);
		pwcMsgEnd += uLen;
	}
	pwcMsgEnd = MCF_wcpcpy(pwcMsgEnd, L"\r\n");
#ifndef NDEBUG
	MCF_wcpcpy(pwcMsgEnd, L"\r\n单击“确定”终止应用程序，单击“取消”调试应用程序。");
#else
	MCF_wcpcpy(pwcMsgEnd, L"\r\n单击“确定”终止应用程序。");
#endif

	DWORD dwExitCode;
	{
		const HANDLE hThread = CreateThread(nullptr, 0, &ThreadProc, awcBuffer, 0, nullptr);
		if(hThread){
			WaitForSingleObject(hThread, INFINITE);
			GetExitCodeThread(hThread, &dwExitCode);
			CloseHandle(hThread);
			goto jDone;
		}
		*pwcMsgEnd = 0;

		const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
		if(hStdErr != INVALID_HANDLE_VALUE){
			DWORD dwMode;
			if(GetConsoleMode(hStdErr, &dwMode)){
				const wchar_t *pwcRead = awcBuffer;
				for(;;){
					const DWORD dwCharsToWrite = (DWORD)(pwcMsgEnd - pwcRead);
					if(dwCharsToWrite == 0){
						break;
					}
					DWORD dwCharsWritten;
					if(!WriteConsoleW(hStdErr, pwcRead, dwCharsToWrite, &dwCharsWritten, nullptr)){
						break;
					}
					pwcRead += dwCharsWritten;
				}
				goto jDone;
			}
		}

		OutputDebugStringW(awcBuffer);
	}
jDone:
	;

#ifndef NDEBUG
	if(dwExitCode == IDOK){
#endif
		TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
		__builtin_unreachable();
#ifndef NDEBUG
	}
#endif
	__asm__ __volatile__("int3 \n");
}

MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}
MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_BailF(const wchar_t *pwszFormat, ...){
	wchar_t awcBuffer[1024 / sizeof(wchar_t)];
	va_list ap;
	va_start(ap, pwszFormat);
	__mingw_vsnwprintf(awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t), pwszFormat, ap);
	va_end(ap);
	DoBail(awcBuffer);
}
