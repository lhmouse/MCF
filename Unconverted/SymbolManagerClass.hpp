// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SYMBOL_MANAGER_CLASS_HPP__
#define __MCF_SYMBOL_MANAGER_CLASS_HPP__

#include "StdMCF.hpp"
#include <dbghelp.h>

namespace MCF {
	class SymbolManagerClass sealed : NO_COPY_OR_ASSIGN {
	private:
		CriticalSection xm_csLock;
		union {
			BYTE xm_abySymbolInfoBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR)];
			SYMBOL_INFO xm_SymbolInfo;
		};
		bool xm_bSymbolsLoaded;
	public:
		explicit SymbolManagerClass(HANDLE hProcess = GetCurrentProcess());
		~SymbolManagerClass();
	public:
		LPCSTR SymbolFromAddr(DWORD dwAddr);
		bool AreSymbolsLoaded() const;
	};
}

#endif
