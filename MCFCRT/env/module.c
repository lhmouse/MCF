// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "mcfwin.h"
#include "mutex.h"
#include "mingw_hacks.h"
#include "fenv.h"
#include "thread_env.h"
#include "static_ctors.h"
#include "../ext/expect.h"
#include <stdlib.h>

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

static _MCFCRT_Mutex           g_vAtExitMutex   = { 0 };
static AtExitCallbackBlock *   g_pAtExitLast    = nullptr;

static void __MCFCRT_PumpAtEndModule(){
	// ISO C++
	// 3.6.3 Termination [basic.start.term]
	// 1 Destructors (12.4) for initialized objects (...)
	// The completions of the destructors for all initialized objects
	// with thread storage duration within that thread are sequenced
	// before the initiation of the destructors of any object with
	// static storage duration. (...)
	__MCFCRT_TlsCleanup();

	for(;;){
		AtExitCallbackBlock *pBlock;
		{
			_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			{
				pBlock = g_pAtExitLast;
				if(pBlock){
					g_pAtExitLast = pBlock->pPrev;
				}
			}
			_MCFCRT_SignalMutex(&g_vAtExitMutex);
		}
		if(!pBlock){
			break;
		}
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

	_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
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
			_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);

			pBlock->pPrev = g_pAtExitLast;
			g_pAtExitLast = pBlock;
		}
		AtExitCallback *const pCallback = pBlock->aCallbacks + ((pBlock->uSize)++);
		pCallback->pfnProc  = pfnProc;
		pCallback->nContext = nContext;
	}
	_MCFCRT_SignalMutex(&g_vAtExitMutex);
	return true;
}

// ld 自动添加此符号。
extern const IMAGE_DOS_HEADER __image_base__ __asm__("__image_base__");

void *_MCFCRT_GetModuleBase(){
	return (void *)&__image_base__;
}

bool _MCFCRT_EnumerateFirstModuleSection(_MCFCRT_ModuleSectionInfo *pInfo){
	if(__image_base__.e_magic != IMAGE_DOS_SIGNATURE){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}
	const IMAGE_NT_HEADERS *const pNtHeaders = (const IMAGE_NT_HEADERS *)((const char *)&__image_base__ + __image_base__.e_lfanew);
	if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}

	pInfo->__vImpl.__pTable = (const char *)&pNtHeaders->OptionalHeader + pNtHeaders->FileHeader.SizeOfOptionalHeader;
	pInfo->__vImpl.__uCount = pNtHeaders->FileHeader.NumberOfSections;
	pInfo->__vImpl.__uNext  = 0;

	return _MCFCRT_EnumerateNextModuleSection(pInfo);
}
bool _MCFCRT_EnumerateNextModuleSection(_MCFCRT_ModuleSectionInfo *pInfo){
	const size_t uIndex = pInfo->__vImpl.__uNext;
	if(uIndex >= pInfo->__vImpl.__uCount){
		SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	pInfo->__vImpl.__uNext = uIndex + 1;

	const IMAGE_SECTION_HEADER *const pHeader = (const IMAGE_SECTION_HEADER *)pInfo->__vImpl.__pTable + uIndex;
	memcpy(pInfo->__achName, pHeader->Name, 8);
	pInfo->__uRawSize = pHeader->SizeOfRawData;
	pInfo->__pBase    = (char *)&__image_base__ + pHeader->VirtualAddress;
	pInfo->__uSize    = pHeader->Misc.VirtualSize;

	return true;
}
