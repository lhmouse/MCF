// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "_pei386_runtime_relocator_common.h"
#include "bail.h"
#include "../ext/alloca.h"

static unsigned GetSectionTable(const IMAGE_SECTION_HEADER **restrict ppSectionTable, HMODULE hModule){
	const IMAGE_DOS_HEADER *const pImageBase = (void *)hModule;
	if(pImageBase->e_magic != IMAGE_DOS_SIGNATURE){
		return 0;
	}
	const IMAGE_NT_HEADERS *const pNtHeaders = (const IMAGE_NT_HEADERS *)((char *)pImageBase + pImageBase->e_lfanew);
	if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE){
		return 0;
	}
	*ppSectionTable = (const IMAGE_SECTION_HEADER *)((const char *)&(pNtHeaders->OptionalHeader) + pNtHeaders->FileHeader.SizeOfOptionalHeader);
	return pNtHeaders->FileHeader.NumberOfSections;
}

typedef struct tagUnprotectedSection {
	void *pVirtualBase;
	size_t uVirtualSize;
	DWORD dwOldProtect;
} UnprotectedSection;

static void UnprotectSections(UnprotectedSection *restrict pUnprotectResults, HMODULE hModule, const IMAGE_SECTION_HEADER *restrict pSectionTable, unsigned uSectionCount){
	for(unsigned uIndex = 0; uIndex < uSectionCount; ++uIndex){
		UnprotectedSection *const pResult = pUnprotectResults + uIndex;
		void *const pVirtualBase = (char *)hModule + pSectionTable[uIndex].VirtualAddress;
		const size_t uVirtualSize = pSectionTable[uIndex].Misc.VirtualSize;

		MEMORY_BASIC_INFORMATION vVirtualInfo;
		if(VirtualQuery(pVirtualBase, &vVirtualInfo, sizeof(vVirtualInfo)) < sizeof(vVirtualInfo)){
			_MCFCRT_Bail(L"VirtualQuery() 失败。");
		}
		DWORD dwOldProtect, dwNewProtect;
		// Protection flags here are only used to determine the corresponding writeable flags.
		// The actual value stored into `pUnprotectResults` is the one returned by `VirtualProtect()`.
		dwOldProtect = vVirtualInfo.Protect & 0xFF;
		// How to mark it writeable?
		switch(dwOldProtect){
		case PAGE_EXECUTE:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
			dwNewProtect = PAGE_EXECUTE_READWRITE;
			break;
		case PAGE_EXECUTE_WRITECOPY:
			dwNewProtect = PAGE_EXECUTE_WRITECOPY;
			break;
		case PAGE_NOACCESS:
			dwNewProtect = PAGE_NOACCESS;
			break;
		case PAGE_READONLY:
		case PAGE_READWRITE:
			dwNewProtect = PAGE_READWRITE;
			break;
		case PAGE_WRITECOPY:
			dwNewProtect = PAGE_WRITECOPY;
			break;
		default:
			dwNewProtect = dwOldProtect;
			break;
		}
		if(dwNewProtect == dwOldProtect){
			// Nothing to do.
			pResult->pVirtualBase = _MCFCRT_NULLPTR;
			continue;
		}
		// Unprotect it.
		if(!VirtualProtect(pVirtualBase, uVirtualSize, dwNewProtect, &dwOldProtect)){
			_MCFCRT_Bail(L"VirtualProtect() 失败。");
		}
		pResult->pVirtualBase = pVirtualBase;
		pResult->uVirtualSize = uVirtualSize;
		pResult->dwOldProtect = dwOldProtect;
	}
}
static void ReprotectSections(const UnprotectedSection *restrict pUnprotectResults, unsigned uSectionCount){
	for(unsigned uIndex = 0; uIndex < uSectionCount; ++uIndex){
		const UnprotectedSection *const pResult = pUnprotectResults + uIndex;
		void *const pVirtualBase = pResult->pVirtualBase;
		const size_t uVirtualSize = pResult->uVirtualSize;

		if(!pVirtualBase){
			// This section was not unprotected. Nothing to do.
			continue;
		}
		// Reprotect it.
		DWORD dwIgnored;
		if(!VirtualProtect(pVirtualBase, uVirtualSize, pResult->dwOldProtect, &dwIgnored)){
			_MCFCRT_Bail(L"VirtualProtect() 失败。");
		}
	}
}

typedef struct tagRelocationElement_v1 {
	DWORD dwOffset;
	DWORD dwTargetRva;
} RelocationElement_v1;

static void ReallyRelocate_v1(HMODULE hModule, const RelocationElement_v1 *pTable, size_t uSize){
	for(size_t uIndex = 0; uIndex < uSize; ++uIndex){
		const DWORD dwOffset = pTable[uIndex].dwOffset;
		void *const pTarget = (char *)hModule + pTable[uIndex].dwTargetRva;
		*(DWORD *)pTarget += dwOffset;
	}
}

typedef struct tagRelocationElement_v2 {
	DWORD dwSymbolRva;
	DWORD dwTargetRva;
	DWORD dwFlags;
} RelocationElement_v2;

static void ReallyRelocate_v2(HMODULE hModule, const RelocationElement_v2 *pTable, size_t uSize){
	for(size_t uIndex = 0; uIndex < uSize; ++uIndex){
		void *const pSymbol = (char *)hModule + pTable[uIndex].dwSymbolRva;
		const INT_PTR n64Offset = (char *)*(void **)pSymbol - (char *)pSymbol;
		void *const pTarget = (char *)hModule + pTable[uIndex].dwTargetRva;
		switch(pTable[uIndex].dwFlags & 0xFF){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
		case 8:
			*(UINT8 *)pTarget += (UINT8)n64Offset;
			break;
		case 16:
			*(UINT16 *)pTarget += (UINT16)n64Offset;
			break;
		case 32:
			*(UINT32 *)pTarget += (UINT32)n64Offset;
			break;
		case 64:
			*(UINT64 *)pTarget += (UINT64)n64Offset;
			break;
#pragma GCC diagnostic pop
		default:
			_MCFCRT_Bail(L"ReallyRelocate_v2() 失败：重定位块大小无效。");
		}
	}
}

void __MCFCRT_pei386_relocator_common(HMODULE hModule, const DWORD *pdwTableBegin, const DWORD *pdwTableEnd){
	const DWORD *pdwTablePayload = _MCFCRT_NULLPTR;
	unsigned uVersion = 0;
	// Guess version of relocation information.
	if((pdwTableEnd - pdwTableBegin >= 2) && ((pdwTableBegin[0] | pdwTableBegin[1]) != 0)){
		pdwTablePayload = pdwTableBegin;
		uVersion = 0;
	} else if(pdwTableEnd - pdwTableBegin >= 3){
		pdwTablePayload = pdwTableBegin + 3;
		uVersion = pdwTableBegin[2];
	}
	if(!pdwTablePayload){
		// No relocation information.
		return;
	}
	// Get section table.
	const IMAGE_SECTION_HEADER *pSectionTable = _MCFCRT_NULLPTR;
	const unsigned uSectionCount = GetSectionTable(&pSectionTable, hModule);
	// Mark all sections writeable before altering them.
	UnprotectedSection *const pUnprotectResults = _MCFCRT_ALLOCA(uSectionCount * sizeof(UnprotectedSection));
	UnprotectSections(pUnprotectResults, hModule, pSectionTable, uSectionCount);
	// Do the relocation.
	switch(uVersion){
	case 0:
		ReallyRelocate_v1(hModule, (const void *)pdwTablePayload, (size_t)(pdwTableEnd - pdwTablePayload) / 2);
		break;
	case 1:
		ReallyRelocate_v2(hModule, (const void *)pdwTablePayload, (size_t)(pdwTableEnd - pdwTablePayload) / 3);
		break;
	default:
		_MCFCRT_Bail(L"__MCFCRT_pei386_relocator_common() 失败：无法识别的重定位表。");
	}
	// Restore all sections.
	ReprotectSections(pUnprotectResults, uSectionCount);
}
