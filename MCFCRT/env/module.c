// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "module.h"
#include "mcfwin.h"
#include "mingw_hacks.h"
#include "heap.h"
#include "heap_dbg.h"
#include "thread.h"
#include "_eh_top.h"
#include "../ext/expect.h"
#include <stdlib.h>

// ld 自动添加此符号。
extern IMAGE_DOS_HEADER g_vDosHeader __asm__("__image_base__");

typedef struct tagAtExitNode {
	struct tagAtExitNode *pNext;

	void (__cdecl *pfnProc)(intptr_t);
	intptr_t nContext;
} AT_EXIT_NODE;

static AT_EXIT_NODE *volatile g_pAtExitHead = NULL;

static unsigned int g_uInitState = 0;

#ifdef __MCF_CRT_HEAPDBG_ON
static inline bool HeapDebugInit(){
	return __MCF_CRT_HeapDbgInit();
}
static inline void HeapDebugUninit(){
	__MCF_CRT_HeapDbgUninit();
}
#else
#	define HeapDebugInit()		(true)
#	define HeapDebugUninit()	((void)0)
#endif

static inline bool DoCtors(){
	extern void __cdecl __main(void);

	__main();
	return true;
}
static inline void DoDtors(){
	AT_EXIT_NODE *pHead = __atomic_exchange_n(&g_pAtExitHead, NULL, __ATOMIC_RELAXED);
	while(pHead){
		(*(pHead->pfnProc))(pHead->nContext);

		AT_EXIT_NODE *const pNext = pHead->pNext;
		free(pHead);
		pHead = pNext;
	}
}

static bool Init(unsigned int *puState){
	switch(*puState){

#define DO_INIT(n, exp)	\
	case (n):	\
		{	\
			if(!(exp)){	\
				return false;	\
			}	\
			++*puState;	\
		}

//=========================================================
	DO_INIT(0, __MCF_CRT_HeapInit());
	DO_INIT(1, HeapDebugInit());
	DO_INIT(2, __MCF_CRT_TlsEnvInit());
	DO_INIT(3, __MCF_CRT_MinGWHacksInit());
	DO_INIT(4, DoCtors());
	DO_INIT(5, __MCF_CRT_ThreadInit());
//=========================================================

		break;
	default:
		__builtin_trap();
	}
	return true;
}
static void Uninit(unsigned int *puState){
	if(*puState == 0){
		return;
	}
	const DWORD dwLastError = GetLastError();
	switch(--*puState){

#define DO_UNINIT(n, exp)	\
	case (n):	\
		{	\
			(exp);	\
			--*puState;	\
		}

//=========================================================
	DO_UNINIT(5, __MCF_CRT_ThreadUninit());
	DO_UNINIT(4, DoDtors());
	DO_UNINIT(3, __MCF_CRT_MinGWHacksUninit());
	DO_UNINIT(2, __MCF_CRT_TlsEnvUninit());
	DO_UNINIT(1, HeapDebugUninit());
	DO_UNINIT(0, __MCF_CRT_HeapUninit());
//=========================================================

		break;
	default:
		__builtin_trap();
	}
	SetLastError(dwLastError);
}

bool __MCF_CRT_BeginModule(void){
	if(!Init(&g_uInitState)){
		Uninit(&g_uInitState);
		return false;
	}
	return true;
}
void __MCF_CRT_EndModule(void){
	Uninit(&g_uInitState);
}

int MCF_CRT_AtEndModule(void (*pfnProc)(intptr_t), intptr_t nContext){
	AT_EXIT_NODE *const pNode = malloc(sizeof(AT_EXIT_NODE));
	if(!pNode){
		return -1;
	}
	pNode->pfnProc = pfnProc;
	pNode->nContext = nContext;

	pNode->pNext = __atomic_load_n(&g_pAtExitHead, __ATOMIC_RELAXED);
	while(EXPECT(!__atomic_compare_exchange_n(
		&g_pAtExitHead, &(pNode->pNext), pNode,
		false, __ATOMIC_RELAXED, __ATOMIC_RELAXED
	))){
		// 空的。
	}

	return 0;
}

void *MCF_CRT_GetModuleBase(){
	return &g_vDosHeader;
}
