// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"

bool MCFCRT_OnDllProcessAttach(void *hDll, bool bDynamic){
	(void)hDll;
	(void)bDynamic;

	return true;
}
void MCFCRT_OnDllProcessDetach(void *hDll, bool bDynamic){
	(void)hDll;
	(void)bDynamic;
}
void MCFCRT_OnDllThreadAttach(void *hDll){
	(void)hDll;
}
void MCFCRT_OnDllThreadDetach(void *hDll){
	(void)hDll;
}
