// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "../env/_pei386_runtime_relocator_common.h"

extern const DWORD __RUNTIME_PSEUDO_RELOC_LIST__[];
extern const DWORD __RUNTIME_PSEUDO_RELOC_LIST_END__[];

void _pei386_runtime_relocator(void){
	static bool s_relocated = false;

	if(s_relocated){
		return;
	}
	__MCFCRT_pei386_relocator_common(_MCFCRT_GetModuleBase(), __RUNTIME_PSEUDO_RELOC_LIST__, __RUNTIME_PSEUDO_RELOC_LIST_END__);
	s_relocated = true;
}
