// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "mcfwin.h"
#include "mingw_hacks.h"
#include "fenv.h"
#include "thread.h"
#include "static_ctors.h"
#include "../ext/expect.h"
#include <stdlib.h>
#include <winnt.h>

typedef struct tagAtExitNode {
	struct tagAtExitNode *pPrev;

	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
} AtExitNode;

static AtExitNode *volatile g_pAtExitHead = nullptr;

static void PumpAtEndModule(){
	// ISO C++
	// 3.6.3 Termination [basic.start.term]
	// 1 Destructors (12.4) for initialized objects (...)
	// The completions of the destructors for all initialized objects
	// with thread storage duration within that thread are sequenced
	// before the initiation of the destructors of any object with
	// static storage duration. (...)
	MCF_CRT_TlsClearAll();

	AtExitNode *pHead = __atomic_exchange_n(&g_pAtExitHead, nullptr, __ATOMIC_RELAXED);
	while(pHead){
		(*(pHead->pfnProc))(pHead->nContext);

		AtExitNode *const pPrev = pHead->pPrev;
		free(pHead);
		pHead = pPrev;
	}
}

static bool __MCF_CRT_StaticObjectsInit(){
	if(!__MCF_CRT_CallStaticCtors()){
		const DWORD dwError = GetLastError();
		PumpAtEndModule();
		SetLastError(dwError);
		return false;
	}
	return true;
}
static void __MCF_CRT_StaticObjectsUninit(){
	PumpAtEndModule();
	__MCF_CRT_CallStaticDtors();
}

bool __MCF_CRT_BeginModule(void){
	__MCF_CRT_FEnvInit();

	if(!__MCF_CRT_TlsEnvInit()){
		return false;
	}
	if(!__MCF_CRT_MinGWHacksInit()){
		const DWORD dwLastError = GetLastError();
		__MCF_CRT_TlsEnvUninit();
		SetLastError(dwLastError);
		return false;
	}
	if(!__MCF_CRT_StaticObjectsInit()){
		const DWORD dwLastError = GetLastError();
		__MCF_CRT_MinGWHacksUninit();
		__MCF_CRT_TlsEnvUninit();
		SetLastError(dwLastError);
		return false;
	}
	return true;
}
void __MCF_CRT_EndModule(void){
	__MCF_CRT_StaticObjectsUninit();
	__MCF_CRT_MinGWHacksUninit();
	__MCF_CRT_TlsEnvUninit();
}

int MCF_CRT_AtEndModule(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	AtExitNode *const pNode = malloc(sizeof(AtExitNode));
	if(!pNode){
		return -1;
	}
	pNode->pfnProc	= pfnProc;
	pNode->nContext	= nContext;

	pNode->pPrev = __atomic_load_n(&g_pAtExitHead, __ATOMIC_RELAXED);
	while(EXPECT(!__atomic_compare_exchange_n(&g_pAtExitHead, &(pNode->pPrev), pNode, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED))){
		// 空的。
	}

	return 0;
}

// ld 自动添加此符号。
extern IMAGE_DOS_HEADER __image_base__ __asm__("__image_base__");

void *MCF_CRT_GetModuleBase(){
	return &__image_base__;
}
bool MCF_CRT_TraverseModuleSections(bool (__cdecl *pfnCallback)(intptr_t, const char [8], void *, size_t), intptr_t nContext){
	if(__image_base__.e_magic != IMAGE_DOS_SIGNATURE){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}
	const PIMAGE_NT_HEADERS32 pNtHeaders = (PIMAGE_NT_HEADERS32)((char *)&__image_base__ + __image_base__.e_lfanew);
	if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}

	const PIMAGE_SECTION_HEADER pSections = (PIMAGE_SECTION_HEADER)((char *)&(pNtHeaders->OptionalHeader) + pNtHeaders->FileHeader.SizeOfOptionalHeader);
	const size_t uSectionCount = pNtHeaders->FileHeader.NumberOfSections;
	for(size_t i = 0; i < uSectionCount; ++i){
		if(!(*pfnCallback)(nContext, (const char *)pSections[i].Name, (char *)&__image_base__ + pSections[i].VirtualAddress, pSections[i].Misc.VirtualSize)){
			SetLastError(ERROR_SUCCESS);
			return false;
		}
	}
	return true;
}
