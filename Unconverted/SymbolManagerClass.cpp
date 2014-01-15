// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "SymbolManagerClass.hpp"
using namespace MCF;

// 构造函数和析构函数。
SymbolManagerClass::SymbolManagerClass(HANDLE hProcess){
	xm_csLock.Enter();
		if(::SymInitialize(hProcess, nullptr, TRUE)){
			xm_bSymbolsLoaded = true;
			xm_SymbolInfo.SizeOfStruct = sizeof(xm_SymbolInfo);
			xm_SymbolInfo.MaxNameLen = MAX_SYM_NAME;
		} else {
			xm_bSymbolsLoaded = false;
		}
	xm_csLock.Leave();
}
SymbolManagerClass::~SymbolManagerClass(){
	xm_csLock.Enter();
		if(xm_bSymbolsLoaded){
			::SymCleanup(::GetCurrentProcess());
		}
	xm_csLock.Leave();
}

// 其他非静态成员函数。
LPCSTR SymbolManagerClass::SymbolFromAddr(DWORD dwAddr){
	LPCSTR pszRet;
	xm_csLock.Enter();
		if(!xm_bSymbolsLoaded){
			pszRet = nullptr;
		} else if(::SymFromAddr(::GetCurrentProcess(), dwAddr, nullptr, &xm_SymbolInfo)){
			pszRet = xm_SymbolInfo.Name;
		} else {
			pszRet = "";
		}
	xm_csLock.Leave();
	return pszRet;
}
bool SymbolManagerClass::AreSymbolsLoaded() const {
	return xm_bSymbolsLoaded;
}
