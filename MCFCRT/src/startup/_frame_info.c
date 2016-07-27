// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "_frame_info.h"
#include "../env/mcfwin.h"
#include "../env/module.h"

typedef struct tagModuleSectionInfo {
	char achName[8];
	_MCFCRT_STD size_t uRawSize;
	void *pBase;
	_MCFCRT_STD size_t uSize;

	struct {
		const void *pTable;
		_MCFCRT_STD size_t uCount;
		_MCFCRT_STD size_t uNext;
	} __vImpl;
} ModuleSectionInfo;

static bool EnumerateNextModuleSection(ModuleSectionInfo *pInfo){
	const size_t uIndex = pInfo->__vImpl.uNext;
	if(uIndex >= pInfo->__vImpl.uCount){
		return false;
	}
	pInfo->__vImpl.uNext = uIndex + 1;

	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	const IMAGE_SECTION_HEADER *const pHeader = (const IMAGE_SECTION_HEADER *)pInfo->__vImpl.pTable + uIndex;
	memcpy(pInfo->achName, pHeader->Name, 8);
	pInfo->uRawSize = pHeader->SizeOfRawData;
	pInfo->pBase    = (char *)pImageBase + pHeader->VirtualAddress;
	pInfo->uSize    = pHeader->Misc.VirtualSize;

	return true;
}
static bool EnumerateFirstModuleSection(ModuleSectionInfo *pInfo){
	const IMAGE_DOS_HEADER *const pImageBase = _MCFCRT_GetModuleBase();
	if(pImageBase->e_magic != IMAGE_DOS_SIGNATURE){
		return false;
	}
	const IMAGE_NT_HEADERS *const pNtHeaders = (const IMAGE_NT_HEADERS *)((char *)pImageBase + pImageBase->e_lfanew);
	if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE){
		return false;
	}

	pInfo->__vImpl.pTable = (const char *)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader;
	pInfo->__vImpl.uCount = pNtHeaders->FileHeader.NumberOfSections;
	pInfo->__vImpl.uNext  = 0;

	return EnumerateNextModuleSection(pInfo);
}

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
// 6 个指针大小应该足够，我们在这里保留到 8 个。
struct object {
	void *impl[8];
};

__attribute__((__weak__))
extern void __register_frame_info(const void *, struct object *);
__attribute__((__weak__))
extern void *__deregister_frame_info(const void *);

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char    g_probe[0] = { };

static const void *  g_eh_base;
static struct object g_eh_object;

static bool RegisterFrameInfo(void){
	if(__register_frame_info){
		const void *base = nullptr;
		ModuleSectionInfo section;
		for(bool valid = EnumerateFirstModuleSection(&section); valid; valid = EnumerateNextModuleSection(&section)){
			const char *const begin = section.pBase;
			const char *const end   = begin + section.uSize;
			if((begin <= g_probe) && (g_probe < end)){
				base = begin;
				break;
			}
		}
		if(!base){
			SetLastError(ERROR_BAD_FORMAT);
			return false;
		}
		__register_frame_info(base, &g_eh_object);
		g_eh_base = base;
	}
	return true;
}
static void DeregisterFrameInfo(void){
	if(__deregister_frame_info){
		const void *const base = g_eh_base;
		if(!base){
			return;
		}
		g_eh_base = nullptr;
		__deregister_frame_info(base);
	}
}

bool __MCFCRT_FrameInfoInit(void){
	if(!RegisterFrameInfo()){
		return false;
	}
	return true;
}
void __MCFCRT_FrameInfoUninit(void){
	DeregisterFrameInfo();
}
