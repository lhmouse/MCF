// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../ext/unref_param.h"

bool MCFDll_OnProcessAttach(void *hDll, bool bDynamic){
	UNREF_PARAM(hDll);
	UNREF_PARAM(bDynamic);

	return true;
}
void MCFDll_OnProcessDetach(void *hDll, bool bDynamic){
	UNREF_PARAM(hDll);
	UNREF_PARAM(bDynamic);
}
void MCFDll_OnThreadAttach(void *hDll){
	UNREF_PARAM(hDll);
}
void MCFDll_OnThreadDetach(void *hDll){
	UNREF_PARAM(hDll);
}
