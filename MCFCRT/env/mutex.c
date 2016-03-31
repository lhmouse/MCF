// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "mutex.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include "../ext/expect.h"
#include <limits.h>
#include <winternl.h>
#include <ntstatus.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtWaitForKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtReleaseKeyedEvent(HANDLE hKeyedEvent, void *pKey, BOOLEAN bAlertable, const LARGE_INTEGER *pliTimeout);

static inline bool atomic_bit_test_and_set_acquire(volatile uintptr_t *p, unsigned bit){
	bool cf;
	__asm__ volatile (
		"xor eax, eax \n"
#ifdef _WIN64
		"lock bts qword ptr[rdx], rcx \n"
#else
		"lock bts dword ptr[edx], ecx\n"
#endif
		"setc %0 \n"
		: "=a"(cf)
		: "d"(p), "c"(bit)
	);
	return cf;
}
static inline bool atomic_bit_test_and_clear_release(volatile uintptr_t *p, unsigned bit){
	bool cf;
	__asm__ volatile (
		"xor eax, eax \n"
#ifdef _WIN64
		"lock btr qword ptr[rdx], rcx \n"
#else
		"lock btr dword ptr[edx], ecx\n"
#endif
		"setc %0 \n"
		: "=a"(cf)
		: "d"(p), "c"(bit)
	);
	return cf;
}

#define FL_LOCKED               ((uintptr_t)0x0001)
#define FL_WAIT_WANTED          ((uintptr_t)0x0002)
#define FL_RESERVED_2           ((uintptr_t)0x0004)
#define FL_RESERVED_3           ((uintptr_t)0x0008)

#define FLAG_BIT_COUNT          ((size_t)4)

#define GET_THREAD_COUNT(c_)    ((size_t)(uintptr_t)(c_) >> FLAG_BIT_COUNT)
#define MAKE_THREAD_COUNT(c_)   ((uintptr_t)(size_t)(c_) << FLAG_BIT_COUNT)

bool _MCFCRT_WaitForMutex(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount, uint64_t u64UntilFastMonoClock){
	while(atomic_bit_test_and_set_acquire(pMutex, 0) == true){
		Sleep(1);
	}
	return true;
}
void _MCFCRT_WaitForMutexForever(_MCFCRT_Mutex *pMutex, size_t uMaxSpinCount){
	// TODO
	_MCFCRT_WaitForMutex(pMutex, uMaxSpinCount, UINT64_MAX);
}
void _MCFCRT_SignalMutex(_MCFCRT_Mutex *pMutex){
	atomic_bit_test_and_clear_release(pMutex, 0);
}
