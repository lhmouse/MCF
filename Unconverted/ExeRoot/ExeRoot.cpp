// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "ExeRoot.hpp"
#include <cstdlib>
#include <vector>
using namespace MCF;

extern "C" void __cdecl __main();

namespace {
	typedef struct xtagAtExitElement {
		void (__cdecl *pfnToCall)();
		xtagAtExitElement *pNext;
	} xAT_EXIT_ELEMENT;

	xAT_EXIT_ELEMENT						*xs_pAtExitListHead		= nullptr;
	CRITICAL_SECTION						xs_csAtExitListLock;

	std::atomic<AbstractExeRootClass *>		xs_pCurrentExeRoot		(nullptr);
}

extern "C" int atexit(void (__cdecl *pfnToCall)()){
	xAT_EXIT_ELEMENT *const pAtExitElement = (xAT_EXIT_ELEMENT *)::HeapAlloc(::GetProcessHeap(), 0, sizeof(xAT_EXIT_ELEMENT));
	if(pAtExitElement == nullptr){
		return 1;
	}

	pAtExitElement->pfnToCall = pfnToCall;

	::EnterCriticalSection(&xs_csAtExitListLock);
		if(xs_pAtExitListHead == nullptr){
			pAtExitElement->pNext = nullptr;
			xs_pAtExitListHead = pAtExitElement;
		} else {
			pAtExitElement->pNext = xs_pAtExitListHead;
			xs_pAtExitListHead = pAtExitElement;
		}
	::LeaveCriticalSection(&xs_csAtExitListLock);

	return 0;
}
static void xDoAtExit(){
	for(;;){
		::EnterCriticalSection(&xs_csAtExitListLock);
			xAT_EXIT_ELEMENT *const pHead = xs_pAtExitListHead;
			if(pHead != nullptr){
				xs_pAtExitListHead = pHead->pNext;
			}
		::LeaveCriticalSection(&xs_csAtExitListLock);

		if(pHead == nullptr){
			break;
		}

		(*pHead->pfnToCall)();
		::HeapFree(::GetProcessHeap(), 0, pHead);
	}
}

extern "C" void __cdecl __MCFStartup(){
	::InitializeCriticalSectionAndSpinCount(&xs_csAtExitListLock, 0x400);

	::__main();

	AbstractExeRootClass *const pExeRoot = AbstractExeRootClass::xs_pLastLatentExeRoot;

	ASSERT(pExeRoot != nullptr);

	const HINSTANCE hInstance = ::GetModuleHandle(nullptr);

	std::vector<LPCTSTR> vecArgVector;
	TString strCmdLine(::GetCommandLine());

	STARTUPINFO StartupInfo;
	::GetStartupInfo(&StartupInfo);
	const int nCmdShow = ((StartupInfo.dwFlags & STARTF_USESHOWWINDOW) != 0) ? (int)StartupInfo.wShowWindow : SW_SHOWDEFAULT;

	LPCTSTR pcchRead = strCmdLine;
	LPTSTR pchWrite = strCmdLine;
	bool bInQuote = false;
	LPCTSTR pstrArgBegin = nullptr;
	for(;;){
		const TCHAR ch = *pcchRead;
		if(ch == 0){
			break;
		}
		switch(ch){
			case _T('"'):
				if(bInQuote){
					if(pcchRead[1] == _T('"')){
						++pcchRead;
						*(pchWrite++) = _T('"');
					} else {
						bInQuote = false;
					}
				} else {
					if(pstrArgBegin == nullptr){
						pstrArgBegin = pchWrite;
					}
					bInQuote = true;
				}
				break;
			case _T(' '):
			case _T('\t'):
				if(!bInQuote && (pstrArgBegin != nullptr)){
					*(pchWrite++) = 0;
					vecArgVector.emplace_back(pstrArgBegin);
					pstrArgBegin = nullptr;
					break;
				}
			default:
				*(pchWrite++) = ch;
				if(pstrArgBegin == nullptr){
					pstrArgBegin = pcchRead;
				}
				break;
		}
		++pcchRead;
	}
	if(pstrArgBegin != nullptr){
		vecArgVector.emplace_back(pstrArgBegin);
	}
	vecArgVector.emplace_back(nullptr);

	int nExitCode = -1;

	for(AbstractExeRootClass *pCurrent = pExeRoot; pCurrent != nullptr; pCurrent = pCurrent->xm_pNext){
		xs_pCurrentExeRoot.exchange(pCurrent);
		pCurrent->xm_nCmdShow = nCmdShow;
		pCurrent->xm_hInstance = hInstance;
		pExeRoot->xm_pvecArgVector = &vecArgVector;

		nExitCode = pCurrent->ParasitizeCurrentThread(false, false);
		if(nExitCode != 0){
			break;
		}
	}

	xDoAtExit();

	::DeleteCriticalSection(&xs_csAtExitListLock);

	::ExitProcess((unsigned int)nExitCode);
}

namespace MCF {
	extern AbstractExeRootClass *GetExeRootPtr(){
		return xs_pCurrentExeRoot;
	}
	extern HINSTANCE GetHInstance(){
		return (xs_pCurrentExeRoot == nullptr) ? NULL : xs_pCurrentExeRoot.load()->GetHInstance();
	}
}

