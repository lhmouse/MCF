// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "ExeRoot.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#include <windows.h>
#include <tchar.h>

// -nostartfiles -lshlwapi -lshell32 -ladvapi32 -luser32 -lkernel32 -Wl,-e__MCFExeStartup

namespace {
	std::vector<LPCTSTR> GetArgV(std::vector<TCHAR> &vecCmdLine){
		std::vector<LPCTSTR> vecRet;

		auto iterWrite = vecCmdLine.begin();
		auto iterRead = vecCmdLine.cbegin();
		bool bInQuote = false;
		LPCTSTR pszArgBegin = nullptr;
		for(;;){
			const TCHAR ch = *iterRead;
			if(ch == 0){
				*iterWrite = 0;
				if(pszArgBegin != nullptr){
					vecRet.push_back(pszArgBegin);
				}
				break;
			}
			switch(ch){
			case _T('\"'):
				if(bInQuote){
					if(*(iterRead + 1) == _T('\"')){
						++iterRead;
						*(iterWrite++) = _T('\"');
					} else {
						bInQuote = false;
					}
				} else {
					bInQuote = true;
					if(pszArgBegin == nullptr){
						pszArgBegin = &*iterWrite;
					}
				}
				break;
			case _T(' '):
			case _T('\t'):
				if(!bInQuote){
					if(pszArgBegin != nullptr){
						*(iterWrite++) = 0;
						vecRet.push_back(pszArgBegin);
						pszArgBegin = nullptr;
					}
					break;
				}
			default:
				*iterWrite = ch;
				if(pszArgBegin == nullptr){
					pszArgBegin = &*iterWrite;
				}
				++iterWrite;
				break;
			}
			++iterRead;
		}
		vecRet.push_back(nullptr);

		return std::move(vecRet);
	}
}

extern "C" __attribute__((noreturn)) void WINAPI __MCFExeStartup(){
	register DWORD dwExitCode;
	::__MCF_CRTBegin();
	{
		const LPCTSTR pszCmdLine = ::GetCommandLine();
		std::vector<TCHAR> vecCmdLineBuffer(pszCmdLine, pszCmdLine + (std::_tcslen(pszCmdLine) + 1));
		const auto vecArgs(GetArgV(vecCmdLineBuffer));
		dwExitCode = ::MCFMain(vecArgs.size() - 1, vecArgs.data());
	}
	::__MCF_CRTEnd();
	::ExitProcess(dwExitCode);
	for(;;);
}

#ifndef __amd64__
extern "C" __attribute__((alias("__MCFExeStartup@0"))) void __cdecl _MCFExeStartup();
#endif
