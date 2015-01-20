// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "module.h"
#include "mcfwin.h"
#include "mingw_hacks.h"
#include "fenv.h"
#include "heap.h"
#include "heap_dbg.h"
#include "thread.h"
#include "hooks.h"
#include "../ext/expect.h"
#include <stdlib.h>

typedef struct tagAtExitNode {
	struct tagAtExitNode *pPrev;

	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
} AtExitNode;

static AtExitNode *volatile g_pAtExitHead = nullptr;
static unsigned g_uInitState = 0;

// weak
void MCF_PreInitModule(){
}

static inline bool BeginModule(){
	extern void __cdecl __main(void);

	MCF_PreInitModule();
	__main();

	return true;
}
static inline void EndModule(){
	// ISO C++
	// 3.6.3 Termination [basic.start.term]
	// 1 Destructors (12.4) for initialized objects (...)
	// The completions of the destructors for all initialized objects
	// with thread storage duration within that thread are sequenced
	// before the initiation of the destructors of any object with
	// static storage duration. (...)
	MCF_CRT_TlsClearAll();

	// libgcc 使用 atexit() 调用全局析构函数。
	AtExitNode *pHead = __atomic_exchange_n(&g_pAtExitHead, nullptr, __ATOMIC_RELAXED);
	while(pHead){
		(*(pHead->pfnProc))(pHead->nContext);

		AtExitNode *const pPrev = pHead->pPrev;
		free(pHead);
		pHead = pPrev;
	}
}

static bool Init(){
	switch(g_uInitState){

#define DO_INIT(n, exp)	\
	case (n):	\
		{	\
			if(!(exp)){	\
				return false;	\
			}	\
			++g_uInitState;	\
		}

//	=========================================================
	DO_INIT(0, __MCF_CRT_FEnvInit());
	DO_INIT(1, __MCF_CRT_HeapInit());
	DO_INIT(2, __MCF_CRT_HeapDbgInit());
	DO_INIT(3, __MCF_CRT_TlsEnvInit());
	DO_INIT(4, __MCF_CRT_MinGWHacksInit());
	DO_INIT(5, BeginModule());
//	=========================================================

		break;
	default:
		__builtin_trap();
	}
	return true;
}
static void Uninit(){
	if(g_uInitState == 0){
		return;
	}
	const DWORD dwLastError = GetLastError();
	switch(--g_uInitState){

#define DO_UNINIT(n, exp)	\
	case (n):	\
		{	\
			(exp);	\
			--g_uInitState;	\
		}

//	=========================================================
	DO_UNINIT(5, EndModule());
	DO_UNINIT(4, __MCF_CRT_MinGWHacksUninit());
	DO_UNINIT(3, __MCF_CRT_TlsEnvUninit());
	DO_UNINIT(2, __MCF_CRT_HeapDbgUninit());
	DO_UNINIT(1, __MCF_CRT_HeapUninit());
	DO_UNINIT(0, __MCF_CRT_FEnvUninit());
//	=========================================================

		break;
	default:
		__builtin_trap();
	}
	SetLastError(dwLastError);
}

bool __MCF_CRT_BeginModule(void){
	if(!Init()){
		Uninit();
		return false;
	}
	return true;
}
void __MCF_CRT_EndModule(void){
	Uninit();
}

int MCF_CRT_AtEndModule(void (__cdecl *pfnProc)(intptr_t), intptr_t nContext){
	AtExitNode *const pNode = malloc(sizeof(AtExitNode));
	if(!pNode){
		return -1;
	}
	pNode->pfnProc	= pfnProc;
	pNode->nContext	= nContext;

	pNode->pPrev = __atomic_load_n(&g_pAtExitHead, __ATOMIC_RELAXED);
	while(EXPECT(!__atomic_compare_exchange_n(
		&g_pAtExitHead, &(pNode->pPrev), pNode, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)))
	{
		// 空的。
	}

	return 0;
}

void *MCF_CRT_GetModuleBase(){
	// ld 自动添加此符号。
	extern IMAGE_DOS_HEADER vDosHeader __asm__("__image_base__");

	return &vDosHeader;
}
