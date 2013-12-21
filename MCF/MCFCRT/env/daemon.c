// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "daemon.h"
#include "../MCFCRT.h"
#include <stdarg.h>
#include <wchar.h>
#include <windows.h>

static volatile BOOL				g_bQuitNow;
static HANDLE						g_hThread;
static CRITICAL_SECTION				g_csMutex;
static __MCF_CRT_DAEMON_CALLBACK	g_pfnCallback;
static intptr_t						g_nCallbackParam;
static HANDLE						g_hCallbackCommitted;
static HANDLE						g_hCallbackComplete;
static intptr_t						g_nCallbackResult;

static DWORD APIENTRY DaemonThreadProc(LPVOID pParam){
	(void)pParam;

	BOOL bQuitNow = FALSE;
	do {
		WaitForSingleObject(g_hCallbackCommitted, INFINITE);
		if(g_bQuitNow != FALSE){
			bQuitNow = TRUE;
		} else {
			g_nCallbackResult = (*g_pfnCallback)(g_nCallbackParam);
		}
		SetEvent(g_hCallbackComplete);
	} while(!bQuitNow);
	return 0;
}

__MCF_CRT_EXTERN unsigned long __MCF_CRTDaemonInitialize(){
	__atomic_store_n(&g_bQuitNow, 0, __ATOMIC_RELEASE);
	g_hThread = CreateThread(NULL, 0, &DaemonThreadProc, NULL, CREATE_SUSPENDED, NULL);
	if(g_hThread == NULL){
		return GetLastError();
	}

	InitializeCriticalSection(&g_csMutex);
	g_hCallbackCommitted	= CreateEventW(NULL, FALSE, FALSE, NULL);
	g_hCallbackComplete		= CreateEventW(NULL, FALSE, FALSE, NULL);

	ResumeThread(g_hThread);

	return ERROR_SUCCESS;
}
__MCF_CRT_EXTERN void __MCF_CRTDaemonUninitialize(){
	__atomic_store_n(&g_bQuitNow, 1, __ATOMIC_RELEASE);
	__MCF_CRTCommitDaemonCallback(NULL, 0);
	WaitForSingleObject(g_hThread, INFINITE);
	CloseHandle(g_hThread);

	CloseHandle(g_hCallbackComplete);
	CloseHandle(g_hCallbackCommitted);
	DeleteCriticalSection(&g_csMutex);
}

__MCF_CRT_EXTERN intptr_t __MCF_CRTCommitDaemonCallback(__MCF_CRT_DAEMON_CALLBACK pfnDaemonCallback, intptr_t nParam){
	EnterCriticalSection(&g_csMutex);
		g_pfnCallback		= pfnDaemonCallback;
		g_nCallbackParam	= nParam;
		SetEvent(g_hCallbackCommitted);

		WaitForSingleObject(g_hCallbackComplete, INFINITE);
		const intptr_t nMsgResult = g_nCallbackResult;
	LeaveCriticalSection(&g_csMutex);
	return nMsgResult;
}

static intptr_t ShowBail(intptr_t nParam){
	const LPCWSTR pwszDescription = (LPCWSTR)nParam;

	wchar_t awchBuffer[1024];
	wchar_t *pwchWrite = _wcscpyout(awchBuffer, L"应用程序异常终止。请联系作者寻求协助。");
	if(pwszDescription != NULL){
		pwchWrite = _wcscpyout(pwchWrite, L"\r\n\r\n错误描述：\r\n");

		size_t uLen = wcslen(pwszDescription);
		const size_t uMax = (size_t)(awchBuffer + sizeof(awchBuffer) / sizeof(awchBuffer[0]) - pwchWrite) - 128;
		if(uLen > uMax){
			uLen = uMax;
		}
		memcpy(pwchWrite, pwszDescription, uLen * sizeof(wchar_t));	// 我们有必要在这个地方拷贝字符串结束符吗？
		pwchWrite += uLen;
	}
#ifndef NDEBUG
	_wcscpyout(pwchWrite, L"\r\n\r\n单击“确定”终止应用程序，单击“取消”调试应用程序。");
	const int nRet = MessageBoxW(NULL, awchBuffer, L"MCF CRT 错误", MB_ICONERROR | MB_OKCANCEL | MB_TASKMODAL);
#else
	_wcscpyout(pwchWrite, L"\r\n\r\n单击“确定”终止应用程序。");
	const int nRet =  MessageBoxW(NULL, awchBuffer, L"MCF CRT 错误", MB_ICONERROR | MB_OK | MB_TASKMODAL);
#endif
	return nRet;
}
static inline __attribute__((always_inline)) void DoBail(const wchar_t *pwszDescription){
	const int __attribute__((unused)) nRet = __MCF_CRTCommitDaemonCallback(&ShowBail, (intptr_t)pwszDescription);
#ifndef NDEBUG
	if(nRet == IDOK){
#endif
		TerminateProcess(GetCurrentProcess(), ERROR_PROCESS_ABORTED);
#ifndef NDEBUG
	}
#endif
	__asm__ __volatile__("int 3 \n");
}

__MCF_CRT_EXTERN void __MCF_Bail(const wchar_t *pwszDescription){
	DoBail(pwszDescription);
}
__MCF_CRT_EXTERN void __MCF_BailF(const wchar_t *pwszFormat, ...){
	wchar_t awchBuffer[1024];
	va_list ap;
	va_start(ap, pwszFormat);
	vswprintf(awchBuffer, sizeof(awchBuffer) / sizeof(wchar_t), pwszFormat, ap);
	va_end(ap);
	DoBail(awchBuffer);
}
__MCF_CRT_EXTERN void __MCF_BailUTF8(const char *pszDescription){
	wchar_t awchBuffer[1024];
	MultiByteToWideChar(CP_UTF8, 0, pszDescription, -1, awchBuffer, sizeof(awchBuffer) / sizeof(wchar_t));
	DoBail(awchBuffer);
}
