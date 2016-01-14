// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../ext/unref_param.h"

bool MCFCRT_OnDllProcessAttach(void *hDll, bool bDynamic){
	UNREF_PARAM(hDll);
	UNREF_PARAM(bDynamic);

	return true;
}
void MCFCRT_OnDllProcessDetach(void *hDll, bool bDynamic){
	UNREF_PARAM(hDll);
	UNREF_PARAM(bDynamic);
}
void MCFCRT_OnDllThreadAttach(void *hDll){
	UNREF_PARAM(hDll);
}
void MCFCRT_OnDllThreadDetach(void *hDll){
	UNREF_PARAM(hDll);
}
