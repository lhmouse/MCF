// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "mcfwin.h"
#include "mutex.h"
#include "mingw_hacks.h"
#include "fenv.h"
#include "thread.h"
#include "static_ctors.h"
#include "../ext/expect.h"
#include <stdlib.h>

enum {
	kMutexSpinCount = 100,
};

typedef struct tagAtExitCallback {
	_MCFCRT_AtEndModuleCallback pfnProc;
	intptr_t nContext;
} AtExitCallback;

enum {
	kAtExitCallbacksPerBlock = 64,
};

typedef struct tagAtExitCallbackBlock {
	struct tagAtExitCallbackBlock *pPrev;

	size_t uSize;
	AtExitCallback aCallbacks[kAtExitCallbacksPerBlock];
} AtExitCallbackBlock;

static _MCFCRT_Mutex           g_vAtExitMutex   = 0;
static AtExitCallbackBlock *   g_pAtExitLast    = nullptr;

static void __MCFCRT_PumpAtEndModule(){
	// ISO C++
	// 3.6.3 Termination [basic.start.term]
	// 1 Destructors (12.4) for initialized objects (...)
	// The completions of the destructors for all initialized objects
	// with thread storage duration within that thread are sequenced
	// before the initiation of the destructors of any object with
	// static storage duration. (...)
	__MCFCRT_TlsThreadCleanup();

	for(;;){
		AtExitCallbackBlock *pBlock;

		_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, kMutexSpinCount);
		{
			pBlock = g_pAtExitLast;
			if(!pBlock){
				_MCFCRT_SignalMutex(&g_vAtExitMutex);
				break;
			}
			g_pAtExitLast = pBlock->pPrev;
		}
		_MCFCRT_SignalMutex(&g_vAtExitMutex);

		for(size_t i = pBlock->uSize; i != 0; --i){
			const AtExitCallback *const pCur = pBlock->aCallbacks + i - 1;
			(*(pCur->pfnProc))(pCur->nContext);
		}
		free(pBlock);
	}
}

static bool __MCFCRT_StaticObjectsInit(){
	if(!__MCFCRT_CallStaticCtors()){
		const DWORD dwError = GetLastError();
		__MCFCRT_PumpAtEndModule();
		SetLastError(dwError);
		return false;
	}
	return true;
}
static void __MCFCRT_StaticObjectsUninit(){
	__MCFCRT_PumpAtEndModule();
	__MCFCRT_CallStaticDtors();
}

bool __MCFCRT_BeginModule(){
	__MCFCRT_FEnvInit();

	if(!__MCFCRT_ThreadEnvInit()){
		return false;
	}
	if(!__MCFCRT_MinGWHacksInit()){
		const DWORD dwLastError = GetLastError();
		__MCFCRT_ThreadEnvUninit();
		SetLastError(dwLastError);
		return false;
	}
	if(!__MCFCRT_StaticObjectsInit()){
		const DWORD dwLastError = GetLastError();
		__MCFCRT_MinGWHacksUninit();
		__MCFCRT_ThreadEnvUninit();
		SetLastError(dwLastError);
		return false;
	}
	return true;
}
void __MCFCRT_EndModule(){
	__MCFCRT_StaticObjectsUninit();
	__MCFCRT_MinGWHacksUninit();
	__MCFCRT_ThreadEnvUninit();
}

bool _MCFCRT_AtEndModule(_MCFCRT_AtEndModuleCallback pfnProc, intptr_t nContext){
	AtExitCallbackBlock *pBlock;

	_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, kMutexSpinCount);
	{
		pBlock = g_pAtExitLast;
		if(!pBlock || (pBlock->uSize >= kAtExitCallbacksPerBlock)){
			_MCFCRT_SignalMutex(&g_vAtExitMutex);
			{
				pBlock = malloc(sizeof(AtExitCallbackBlock));
				if(!pBlock){
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return false;
				}
				pBlock->uSize = 0;
			}
			_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, kMutexSpinCount);

			pBlock->pPrev = g_pAtExitLast;
			g_pAtExitLast = pBlock;
		}
		pBlock->aCallbacks[pBlock->uSize].pfnProc  = pfnProc;
		pBlock->aCallbacks[pBlock->uSize].nContext = nContext;
		++(pBlock->uSize);
	}
	_MCFCRT_SignalMutex(&g_vAtExitMutex);
	return true;
}

// ld 自动添加此符号。
extern IMAGE_DOS_HEADER __image_base__ __asm__("__image_base__");

void *_MCFCRT_GetModuleBase(){
	return &__image_base__;
}

bool _MCFCRT_TraverseModuleSections(_MCFCRT_TraverseModuleSectionsCallback pfnCallback, intptr_t nContext){
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
