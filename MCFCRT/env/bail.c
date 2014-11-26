// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "bail.h"
#include "mcfwin.h"
#include "../ext/strcpyout.h"
#include <stdarg.h>
#include <wchar.h>
#include <stdlib.h>

static DWORD APIENTRY ThreadProc(LPVOID pParam){
	const LPCWSTR pwszDescription = (LPCWSTR)pParam;

	wchar_t awcBuffer[2049];
	wchar_t *pwcWrite = MCF_wcscpyout(awcBuffer, L"应用程序异常终止。请联系作者寻求协助。");
	if(pwszDescription){
		pwcWrite = MCF_wcscpyout(pwcWrite, L"\r\n\r\n错误描述：\r\n");

		size_t uLen = wcslen(pwszDescription);
		const wchar_t *const pwcEnd = awcBuffer + sizeof(awcBuffer) / sizeof(awcBuffer[0]);
		const size_t uMax = (size_t)(pwcEnd - pwcWrite) - 64; // 后面还有一些内容，保留一些字符。
		if(uLen > uMax){
			uLen = uMax;
		}
		memcpy(pwcWrite, pwszDescription, uLen * sizeof(wchar_t)); // 我们有必要在这个地方拷贝字符串结束符吗？
		pwcWrite += uLen;
	}
#ifndef NDEBUG
	MCF_wcscpyout(pwcWrite, L"\r\n\r\n单击“确定”终止应用程序，单击“取消”调试应用程序。");
	const int nRet = MessageBoxW(NULL, awcBuffer, L"MCF CRT 错误", MB_ICONERROR | MB_OKCANCEL | MB_TASKMODAL | MB_DEFBUTTON2);
#else
	MCF_wcscpyout(pwcWrite, L"\r\n\r\n单击“确定”终止应用程序。");
	const int nRet =  MessageBoxW(NULL, awcBuffer, L"MCF CRT 错误", MB_ICONERROR | MB_OK | MB_TASKMODAL);
#endif
	return (DWORD)nRet;
}
MCF_CRT_NORETURN_IF_NDEBUG static void DoBail(const wchar_t *pwszDescription){
	DWORD dwExitCode = IDCANCEL;
	const HANDLE hThread = CreateThread(NULL, 0, &ThreadProc, (LPVOID)pwszDescription, 0, NULL);
	if(hThread){
		WaitForSingleObject(hThread, INFINITE);
		GetExitCodeThread(hThread, &dwExitCode);
		CloseHandle(hThread);
	} else {
		__asm__ __volatile__("int 3 \n");
	}

#ifndef NDEBUG
	if(dwExitCode == IDOK){
#endif
		TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
		__builtin_unreachable();
#ifndef NDEBUG
	}
#endif
	__asm__ __volatile__("int 3 \n");
}

MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}
MCF_CRT_NORETURN_IF_NDEBUG void MCF_CRT_BailF(const wchar_t *pwszFormat, ...){
	wchar_t awcBuffer[1025];
	va_list ap;
	va_start(ap, pwszFormat);
	__mingw_vsnwprintf(awcBuffer, sizeof(awcBuffer) / sizeof(wchar_t), pwszFormat, ap);
	va_end(ap);
	DoBail(awcBuffer);
}
