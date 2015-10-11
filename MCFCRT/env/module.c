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

typedef struct tagAtExitCallback {
	void (*pfnProc)(intptr_t);
	intptr_t nContext;
} AtExitCallback;

enum {
	kAtExitCallbacksPerBlock = 64,
};

typedef struct tagAtExitCallbackBlock {
	struct tagAtExitCallbackBlock *pNextBlock;

	size_t uSize;
	AtExitCallback aCallbacks[kAtExitCallbacksPerBlock];
} AtExitCallbackBlock;

static SRWLOCK               g_srwlAtExitMutex   = SRWLOCK_INIT;
static AtExitCallbackBlock * g_pAtExitFirst      = nullptr;
static AtExitCallbackBlock * g_pAtExitLast       = nullptr;

static void __MCF_CRT_PumpAtEndModule(){
	// ISO C++
	// 3.6.3 Termination [basic.start.term]
	// 1 Destructors (12.4) for initialized objects (...)
	// The completions of the destructors for all initialized objects
	// with thread storage duration within that thread are sequenced
	// before the initiation of the destructors of any object with
	// static storage duration. (...)
	__MCF_CRT_TlsThreadCleanup();

	for(;;){
		AtExitCallbackBlock *pBlock;

		AcquireSRWLockExclusive(&g_srwlAtExitMutex);
		{
			pBlock = g_pAtExitFirst;
			if(!pBlock){
				ReleaseSRWLockExclusive(&g_srwlAtExitMutex);
				break;
			}

			AtExitCallbackBlock *const pNextBlock = pBlock->pNextBlock;
			g_pAtExitFirst = pNextBlock;
			if(pNextBlock){
				// 单向链表。
			} else {
				g_pAtExitLast = nullptr;
			}
		}
		ReleaseSRWLockExclusive(&g_srwlAtExitMutex);

		const AtExitCallback *pCur = pBlock->aCallbacks, *const pEnd = pBlock->aCallbacks + pBlock->uSize;
		while(pCur != pEnd){
			(*(pCur->pfnProc))(pCur->nContext);
			++pCur;
		}
		free(pBlock);
	}
}

static bool __MCF_CRT_StaticObjectsInit(){
	if(!__MCF_CRT_CallStaticCtors()){
		const DWORD dwError = GetLastError();
		__MCF_CRT_PumpAtEndModule();
		SetLastError(dwError);
		return false;
	}
	return true;
}
static void __MCF_CRT_StaticObjectsUninit(){
	__MCF_CRT_PumpAtEndModule();
	__MCF_CRT_CallStaticDtors();
}

bool __MCF_CRT_BeginModule(){
	__MCF_CRT_FEnvInit();

	DWORD dwLastError;
	if(!__MCF_CRT_ThreadEnvInit()){
//		dwLastError = GetLastError();
		goto jFailed0;
	}
	if(!__MCF_CRT_MinGWHacksInit()){
		dwLastError = GetLastError();
		goto jFailed1;
	}
	if(!__MCF_CRT_StaticObjectsInit()){
		dwLastError = GetLastError();
		goto jFailed2;
	}
	return true;

//	__MCF_CRT_StaticObjectsUninit();
jFailed2:
	__MCF_CRT_MinGWHacksUninit();
jFailed1:
	__MCF_CRT_ThreadEnvUninit();
	SetLastError(dwLastError);
jFailed0:
	return false;
}
void __MCF_CRT_EndModule(){
	__MCF_CRT_StaticObjectsUninit();
	__MCF_CRT_MinGWHacksUninit();
	__MCF_CRT_ThreadEnvUninit();
}

bool MCF_CRT_AtEndModule(void (*pfnProc)(intptr_t), intptr_t nContext){
	AtExitCallbackBlock *pBlock;

	AcquireSRWLockExclusive(&g_srwlAtExitMutex);
	{
		pBlock = g_pAtExitFirst;
		if(!pBlock || (pBlock->uSize >= kAtExitCallbacksPerBlock)){
			pBlock = malloc(sizeof(AtExitCallbackBlock));
			if(!pBlock){
				ReleaseSRWLockExclusive(&g_srwlAtExitMutex);
				goto jFailed;
			}
			pBlock->pNextBlock = nullptr;
			pBlock->uSize      = 0;

			AtExitCallbackBlock *const pLastBlock = g_pAtExitLast;
			if(pLastBlock){
				pLastBlock->pNextBlock = pBlock;
			} else {
				g_pAtExitFirst = pBlock;
			}
			g_pAtExitLast = pBlock;
		}

		AtExitCallback *const pCur = pBlock->aCallbacks + pBlock->uSize;
		pCur->pfnProc  = pfnProc;
		pCur->nContext = nContext;
		++(pBlock->uSize);
	}
	ReleaseSRWLockExclusive(&g_srwlAtExitMutex);
	return true;

jFailed:
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	return false;
}

// ld 自动添加此符号。
extern IMAGE_DOS_HEADER __image_base__ __asm__("__image_base__");

void *MCF_CRT_GetModuleBase(){
	return &__image_base__;
}
bool MCF_CRT_TraverseModuleSections(bool (*pfnCallback)(intptr_t, const char [8], void *, size_t), intptr_t nContext){
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
