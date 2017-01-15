// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "module.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"

static size_t GetAllSections(const void **ppSectionTable){
	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	if(pImageBase->e_magic != IMAGE_DOS_SIGNATURE){
		return 0;
	}
	const IMAGE_NT_HEADERS *const pNtHeaders = (const IMAGE_NT_HEADERS *)((char *)pImageBase + pImageBase->e_lfanew);
	if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE){
		return 0;
	}
	*ppSectionTable = (const char *)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader;
	return pNtHeaders->FileHeader.NumberOfSections;
}

typedef struct tagUnprotectedSection {
	void *pBase;
	size_t uSize;
	DWORD dwOldProtect;
} UnprotectedSection;

static void UnprotectAllSections(UnprotectedSection *restrict pProtectedSections, const void *restrict pSectionTable, size_t uSectionCount){
	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	for(size_t i = 0; i < uSectionCount; ++i){
		UnprotectedSection *const pUnprotected = pProtectedSections + i;
		const IMAGE_SECTION_HEADER *const pHeader = (const IMAGE_SECTION_HEADER *)pSectionTable + i;
		void *const pBase = (char *)pImageBase + pHeader->VirtualAddress;
		const size_t uSize = pHeader->Misc.VirtualSize;

		MEMORY_BASIC_INFORMATION vMemInfo;
		if(VirtualQuery(pBase, &vMemInfo, sizeof(vMemInfo)) < sizeof(vMemInfo)){
			_MCFCRT_Bail(L"VirtualQuery() 失败。");
		}
		if((vMemInfo.Protect == PAGE_READWRITE) || (vMemInfo.Protect == PAGE_EXECUTE_READWRITE)){
			pUnprotected->pBase = _MCFCRT_NULLPTR;
			pUnprotected->uSize = 0;
		} else {
			if(!VirtualProtect(pBase, uSize, PAGE_EXECUTE_READWRITE, &(pUnprotected->dwOldProtect))){
				_MCFCRT_Bail(L"VirtualProtect() 失败。");
			}
			pUnprotected->pBase = pBase;
			pUnprotected->uSize = uSize;
		}
	}
}
static void ReprotectAllSections(const UnprotectedSection *restrict pProtectedSections, size_t uSectionCount){
	for(size_t i = 0; i < uSectionCount; ++i){
		const UnprotectedSection *const pUnprotected = pProtectedSections + i;
		void *const pBase = pUnprotected->pBase;
		const size_t uSize = pUnprotected->uSize;

		if(pBase){
			DWORD dwUnused;
			if(!VirtualProtect(pBase, uSize, pUnprotected->dwOldProtect, &dwUnused)){
				_MCFCRT_Bail(L"VirtualProtect() 失败。");
			}
		}
	}
}

#define WHY_DO_WE_NEED_INTEGER_PROMOTION(type_, ptr_, add_)      (*(type_ *)(ptr_) = (type_)(*(type_ *)(ptr_) + (type_)(add_)))

static void RealRelocateV1(const DWORD *pdwTable, const DWORD *pdwTableEnd){
	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	for(const DWORD *pdwBase = pdwTable; pdwBase < pdwTableEnd; pdwBase += 2){
		const DWORD dwAddend = pdwBase[0];
		void *const pTarget = (char *)pImageBase + pdwBase[1];

		WHY_DO_WE_NEED_INTEGER_PROMOTION(DWORD, pTarget, dwAddend);
	}
}

static void RealRelocateV2(const DWORD *pdwTable, const DWORD *pdwTableEnd){
	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	for(const DWORD *pdwBase = pdwTable; pdwBase < pdwTableEnd; pdwBase += 3){
		const void *const pSymbol = (char *)pImageBase + pdwBase[0];
		const ptrdiff_t nAddend = (char *)*(void **)pSymbol - (char *)pSymbol;
		void *const pTarget = (char *)pImageBase + pdwBase[1];
		const DWORD dwFlags = pdwBase[2];

		const unsigned uBlockSize = dwFlags & 0xFF;
		if(uBlockSize == 8){
			WHY_DO_WE_NEED_INTEGER_PROMOTION(UINT8 , pTarget, nAddend);
		} else if(uBlockSize == 16){
			WHY_DO_WE_NEED_INTEGER_PROMOTION(UINT16, pTarget, nAddend);
		} else if(uBlockSize == 32){
			WHY_DO_WE_NEED_INTEGER_PROMOTION(UINT32, pTarget, nAddend);
		} else if(uBlockSize == 64){
			WHY_DO_WE_NEED_INTEGER_PROMOTION(UINT64, pTarget, nAddend);
		} else {
			_MCFCRT_Bail(L"RealRelocateV2() 失败：重定位块大小无效。");
		}
	}
}

extern DWORD __RUNTIME_PSEUDO_RELOC_LIST__;
extern DWORD __RUNTIME_PSEUDO_RELOC_LIST_END__;

void _pei386_runtime_relocator(void){
	static bool s_bRelocated = false;

	if(s_bRelocated){
		return;
	}
	s_bRelocated = true;

	const DWORD *const pdwBegin = &__RUNTIME_PSEUDO_RELOC_LIST__;
	const DWORD *const pdwEnd   = &__RUNTIME_PSEUDO_RELOC_LIST_END__;
	const size_t uDwordCount    = (size_t)(pdwEnd - pdwBegin);

	const DWORD *pdwTable = _MCFCRT_NULLPTR;
	unsigned uVersion = 0;

	if((uDwordCount >= 2) && ((pdwBegin[0] != 0) || (pdwBegin[1] != 0))){
		pdwTable = pdwBegin;
		uVersion = 0;
	} else if(uDwordCount >= 3){
		pdwTable = pdwBegin + 3;
		uVersion = pdwBegin[2];
	}
	if(!pdwTable){
		return;
	}

	const void *pSectionTable = _MCFCRT_NULLPTR;
	const size_t uSections = GetAllSections(&pSectionTable);
	UnprotectedSection aUnprotected[uSections];
	UnprotectAllSections(aUnprotected, pSectionTable, uSections);
	{
		if(uVersion == 0){
			RealRelocateV1(pdwTable, pdwEnd);
		} else if(uVersion == 1){
			RealRelocateV2(pdwTable, pdwEnd);
		} else {
			_MCFCRT_Bail(L"_pei386_runtime_relocator() 失败：无法识别的重定位表。");
		}
	}
	ReprotectAllSections(aUnprotected, uSections);
}
