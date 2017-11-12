// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../ext/alloca.h"

static size_t GetAllSections(const IMAGE_SECTION_HEADER **sections_ret){
	const IMAGE_DOS_HEADER *const img_base = _MCFCRT_GetModuleBase();
	if(img_base->e_magic != IMAGE_DOS_SIGNATURE){
		return 0;
	}
	const IMAGE_NT_HEADERS *const nt_hdr = (const IMAGE_NT_HEADERS *)((char *)img_base + img_base->e_lfanew);
	if(nt_hdr->Signature != IMAGE_NT_SIGNATURE){
		return 0;
	}
	*sections_ret = (const IMAGE_SECTION_HEADER *)((const char *)&(nt_hdr->OptionalHeader) + nt_hdr->FileHeader.SizeOfOptionalHeader);
	return nt_hdr->FileHeader.NumberOfSections;
}

typedef struct tagUnprotectResult {
	void *base;
	size_t size;
	DWORD protect_old;
} UnprotectResult;

static void UnprotectSections(UnprotectResult *restrict results, const IMAGE_SECTION_HEADER *sections, size_t count){
	const IMAGE_DOS_HEADER *const img_base = _MCFCRT_GetModuleBase();
	for(size_t i = 0; i < count; ++i){
		void *const base = (char *)img_base + sections[i].VirtualAddress;
		const size_t size = sections[i].Misc.VirtualSize;

		MEMORY_BASIC_INFORMATION info;
		if(VirtualQuery(base, &info, sizeof(info)) < sizeof(info)){
			_MCFCRT_Bail(L"VirtualQuery() 失败。");
		}
		DWORD protect_old = info.Protect & 0xFF;
		DWORD protect_new;
		switch(protect_old){
		case PAGE_EXECUTE:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
			protect_new = PAGE_EXECUTE_READWRITE;
			break;
		case PAGE_EXECUTE_WRITECOPY:
			protect_new = PAGE_EXECUTE_WRITECOPY;
			break;
		case PAGE_NOACCESS:
			protect_new = PAGE_NOACCESS;
			break;
		case PAGE_READONLY:
		case PAGE_READWRITE:
			protect_new = PAGE_READWRITE;
			break;
		case PAGE_WRITECOPY:
			protect_new = PAGE_WRITECOPY;
			break;
		default:
			protect_new = PAGE_NOACCESS;
			break;
		}
		if(protect_new == protect_old){
			results[i].base = _MCFCRT_NULLPTR;
			continue;
		}
		if(!VirtualProtect(base, size, protect_new, &protect_old)){
			_MCFCRT_Bail(L"VirtualProtect() 失败。");
		}
		results[i].base        = base;
		results[i].size        = size;
		results[i].protect_old = protect_old;
	}
}

static void ReprotectSections(const UnprotectResult *restrict results, size_t count){
	for(size_t i = 0; i < count; ++i){
		void *const base = results[i].base;
		const size_t size = results[i].size;

		if(!base){
			continue;
		}
		DWORD protect_old;
		DWORD protect_new = results[i].protect_old;
		if(!VirtualProtect(base, size, protect_new, &protect_old)){
			_MCFCRT_Bail(L"VirtualProtect() 失败。");
		}
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

static void ReallyRelocate_v1(const DWORD *table, const DWORD *end){
	const IMAGE_DOS_HEADER *const img_base = _MCFCRT_GetModuleBase();
	for(const DWORD *cur = table; cur < end; cur += 2){
		const DWORD add = cur[0];
		void *const target = (char *)img_base + cur[1];
		*(DWORD *)target += add;
	}
}

static void ReallyRelocate_v2(const DWORD *table, const DWORD *end){
	const IMAGE_DOS_HEADER *const img_base = _MCFCRT_GetModuleBase();
	for(const DWORD *cur = table; cur < end; cur += 3){
		const void *const symbol = (char *)img_base + cur[0];
		const ptrdiff_t add = (char *)*(void **)symbol - (char *)symbol;
		void *const target = (char *)img_base + cur[1];
		const DWORD flags = cur[2];

		switch(flags & 0xFF){
		case 8:
			*(UINT8 *)target += (UINT8)add;
			break;
		case 16:
			*(UINT16 *)target += (UINT16)add;
			break;
		case 32:
			*(UINT32 *)target += (UINT32)add;
			break;
		case 64:
			*(UINT64 *)target += (UINT64)add;
			break;
		default:
			_MCFCRT_Bail(L"ReallyRelocate_v2() 失败：重定位块大小无效。");
		}
	}
}

#pragma GCC diagnostic pop

extern const DWORD __RUNTIME_PSEUDO_RELOC_LIST__[];
extern const DWORD __RUNTIME_PSEUDO_RELOC_LIST_END__[];

void _pei386_runtime_relocator(void){
	static bool s_relocated = false;

	if(s_relocated){
		return;
	}
	s_relocated = true;

	const DWORD *table = _MCFCRT_NULLPTR;
	unsigned version = 0;

	const DWORD *const begin = __RUNTIME_PSEUDO_RELOC_LIST__;
	const DWORD *const end   = __RUNTIME_PSEUDO_RELOC_LIST_END__;
	if((end - begin >= 2) && ((begin[0] != 0) || (begin[1] != 0))){
		table = begin;
		version = 0;
	} else if(end - begin >= 3){
		table = begin + 3;
		version = begin[2];
	}
	if(!table){
		return;
	}

	const IMAGE_SECTION_HEADER *sections;
	const size_t count = GetAllSections(&sections);
	UnprotectResult *const results = _MCFCRT_ALLOCA(count * sizeof(UnprotectResult));
	UnprotectSections(results, sections, count);
	switch(version){
	case 0:
		ReallyRelocate_v1(table, end);
		break;
	case 1:
		ReallyRelocate_v2(table, end);
		break;
	default:
		_MCFCRT_Bail(L"_pei386_runtime_relocator() 失败：无法识别的重定位表。");
	}
	ReprotectSections(results, count);
}
