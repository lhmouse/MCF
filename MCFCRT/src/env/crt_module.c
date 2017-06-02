// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "crt_module.h"
#include "_atexit_queue.h"
#include "mcfwin.h"
#include "bail.h"

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

static __MCFCRT_AtExitQueue g_vAtQuickExitQueue = { 0 };

static void Dispose_at_quick_exit_queue(void){
	__MCFCRT_AtExitElement vElement;
	while(__MCFCRT_AtExitQueuePop(&vElement, &g_vAtQuickExitQueue)){
		__MCFCRT_AtExitQueueInvoke(&vElement);
	}
}

bool _MCFCRT_AtCrtModuleQuickExit(_MCFCRT_AtCrtModuleExitCallback pfnProc, intptr_t nContext){
	__MCFCRT_AtExitElement vElement = { pfnProc, nContext };
	return __MCFCRT_AtExitQueuePush(&g_vAtQuickExitQueue, &vElement);
}

void __MCFCRT_DiscardCrtModuleQuickExitCallbacks(void){
	__MCFCRT_AtExitQueueClear(&g_vAtQuickExitQueue);
}

static volatile DWORD s_dwExitingThreadId = 0;

static void CheckExitingThread(unsigned uExitCode){
	const DWORD dwCurrentThreadId = GetCurrentThreadId();
	DWORD dwOldExitingThreadId = 0;
	__atomic_compare_exchange_n(&s_dwExitingThreadId, &dwOldExitingThreadId, dwCurrentThreadId, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
	if(dwOldExitingThreadId != 0){
		if(dwOldExitingThreadId == dwCurrentThreadId){
			_MCFCRT_Bail(L"_MCFCRT_QuickExit() 或 _MCFCRT_Exit() 被递归调用。\n"
				"这可能是在被 static 或 thread_local 修饰的对象的析构函数中调用了 exit() 或 quick_exit() 导致的。");
		}
		ExitThread(uExitCode);
		__builtin_trap();
	}
}

_Noreturn void _MCFCRT_ImmediateExit(unsigned uExitCode){
	TerminateProcess(GetCurrentProcess(), uExitCode);
	__builtin_trap();
}
_Noreturn void _MCFCRT_QuickExit(unsigned uExitCode){
	CheckExitingThread(uExitCode);
	Dispose_at_quick_exit_queue();
	_MCFCRT_ImmediateExit(uExitCode);
	__builtin_trap();
}
_Noreturn void _MCFCRT_Exit(unsigned uExitCode){
	CheckExitingThread(uExitCode);
	ExitProcess(uExitCode);
	__builtin_trap();
}
