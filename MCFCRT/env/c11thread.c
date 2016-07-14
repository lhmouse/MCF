// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#define __MCFCRT_C11THREAD_INLINE_OR_EXTERN     extern inline
#include "c11thread.h"
#include "avl_tree.h"
#include "heap.h"
#include "mcfwin.h"
#include "_seh_top.h"

void __MCFCRT_C11threadTlsDestructor(intptr_t context, void *storage){
	void (*const destructor)(void *) = (void (*)(void *))context;

	void *const value = *(void **)storage;
	if(!value){
		return;
	}
	*(void **)storage = nullptr;

	(*destructor)(value);
}

intptr_t __MCFCRT_C11threadUnlockCallback(intptr_t context){
	mtx_t *const mutex_c = (mtx_t *)context;

	if(mutex_c->__mask & mtx_recursive){
		_MCFCRT_ASSERT(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(mutex_c->__owner), __ATOMIC_RELAXED));

		const size_t old_count = mutex_c->__count;
		mutex_c->__count = 0;
		__atomic_store_n(&(mutex_c->__owner), 0, __ATOMIC_RELAXED);

		_MCFCRT_SignalMutex(&(mutex_c->__mutex));
		return (intptr_t)old_count;
	} else {
		_MCFCRT_SignalMutex(&(mutex_c->__mutex));
		return 1;
	}
}
void __MCFCRT_C11threadRelockCallback(intptr_t context, intptr_t unlocked){
	mtx_t *const mutex_c = (mtx_t *)context;

	if(mutex_c->__mask & mtx_recursive){
		_MCFCRT_ASSERT((size_t)unlocked >= 1);
		_MCFCRT_WaitForMutexForever(&(mutex_c->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);

		const uintptr_t self = _MCFCRT_GetCurrentThreadId();
		__atomic_store_n(&(mutex_c->__owner), self, __ATOMIC_RELAXED);
		mutex_c->__count = (size_t)unlocked;
	} else {
		_MCFCRT_ASSERT((size_t)unlocked == 1);
		_MCFCRT_WaitForMutexForever(&(mutex_c->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	}
}

static _MCFCRT_Mutex     g_ctrlmap_mutex = { 0 };
static _MCFCRT_AvlRoot   g_ctrlmap       = nullptr;

typedef enum tagThreadState {
	kStateJoinable,
	kStateZombie,
	kStateJoining,
	kStateJoined,
	kStateDetached,
} ThreadState;

typedef struct tagThreadControl {
	_MCFCRT_AvlNodeHeader header;

	int (*proc)(void *);
	void *param;

	ThreadState state;
	_MCFCRT_ConditionVariable termination;

	uintptr_t tid;
	_MCFCRT_ThreadHandle handle;
} ThreadControl;

static inline int ThreadControlComparatorNodeKey(const _MCFCRT_AvlNodeHeader *lhs, intptr_t rhs){
	const uintptr_t u1 = (uintptr_t)(((const ThreadControl *)lhs)->tid);
	const uintptr_t u2 = (uintptr_t)rhs;
	if(u1 != u2){
		return (u1 < u2) ? -1 : 1;
	}
	return 0;
}
static inline int ThreadControlComparatorNodes(const _MCFCRT_AvlNodeHeader *lhs, const _MCFCRT_AvlNodeHeader *rhs){
	return ThreadControlComparatorNodeKey(lhs, (intptr_t)(((const ThreadControl *)rhs)->tid));
}

static intptr_t UnlockCallbackNative(intptr_t context){
	_MCFCRT_Mutex *const mutex = (_MCFCRT_Mutex *)context;

	_MCFCRT_SignalMutex(mutex);
	return 1;
}
static void RelockCallbackNative(intptr_t context, intptr_t unlocked){
	_MCFCRT_Mutex *const mutex = (_MCFCRT_Mutex *)context;

	_MCFCRT_ASSERT((size_t)unlocked == 1);
	_MCFCRT_WaitForMutexForever(mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
}

__MCFCRT_C_STDCALL __attribute__((__section__(".text$__MCFCRT")))
static unsigned long C11threadProc(void *ctrl_ptr){
	ThreadControl *const ctrl = ctrl_ptr;
	_MCFCRT_ASSERT(ctrl);

	int (*const proc)(void *) = ctrl->proc;
	void *const param = ctrl->param;

	int exit_code;

	__MCFCRT_SEH_TOP_BEGIN
	{
		exit_code = (*proc)(param);
	}
	__MCFCRT_SEH_TOP_END

	_MCFCRT_WaitForMutexForever(&g_ctrlmap_mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		switch(ctrl->state){
		case kStateJoinable:
			ctrl->state = kStateZombie;
			break;
		case kStateZombie:
			_MCFCRT_ASSERT(false);
		case kStateJoining:
			ctrl->state = kStateJoined;
			_MCFCRT_BroadcastConditionVariable(&(ctrl->termination));
			break;
		case kStateJoined:
			_MCFCRT_ASSERT(false);
		case kStateDetached:
			ctrl->state = kStateJoined;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_CloseThread(ctrl->handle);
			_MCFCRT_free(ctrl);
			break;
		default:
			_MCFCRT_ASSERT(false);
		}
	}
	_MCFCRT_SignalMutex(&g_ctrlmap_mutex);

	return (unsigned long)exit_code;
}

uintptr_t __MCFCRT_C11threadCreateJoinable(int (*proc)(void *), void *param){
	ThreadControl *const ctrl = _MCFCRT_malloc(sizeof(ThreadControl));
	if(!ctrl){
		return 0;
	}
	ctrl->proc      = proc;
	ctrl->param     = param;
	ctrl->state     = kStateJoinable;
	_MCFCRT_InitializeConditionVariable(&(ctrl->termination));

	uintptr_t tid;
	const _MCFCRT_ThreadHandle handle = _MCFCRT_CreateNativeThread(&C11threadProc, ctrl, true, &tid);
	if(!handle){
		_MCFCRT_free(ctrl);
		return 0;
	}
	ctrl->tid    = tid;
	ctrl->handle = handle;

	_MCFCRT_WaitForMutexForever(&g_ctrlmap_mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		_MCFCRT_AvlAttach(&g_ctrlmap, (_MCFCRT_AvlNodeHeader *)ctrl, &ThreadControlComparatorNodes);
	}
	_MCFCRT_SignalMutex(&g_ctrlmap_mutex);

	_MCFCRT_ResumeThread(handle);
	return tid;
}
void __MCFCRT_C11threadExit(int exit_code){
	ExitThread((DWORD)exit_code);
}
bool __MCFCRT_C11threadJoin(uintptr_t tid, int *restrict exit_code_ret){
	bool joined = false;

	_MCFCRT_WaitForMutexForever(&g_ctrlmap_mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		ThreadControl *const ctrl = (ThreadControl *)_MCFCRT_AvlFind(&g_ctrlmap, (intptr_t)tid, &ThreadControlComparatorNodeKey);
		if(!ctrl){
			goto done;
		}
		switch(ctrl->state){
		case kStateJoinable:
			ctrl->state = kStateJoining;
			do {
				_MCFCRT_WaitForConditionVariableForever(&(ctrl->termination), &UnlockCallbackNative, &RelockCallbackNative, (intptr_t)&g_ctrlmap_mutex);
			} while(ctrl->state != kStateJoined);
			joined = true;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_WaitForThreadForever(ctrl->handle);
			if(exit_code_ret){
				unsigned long exit_code_dw;
				if(!GetExitCodeThread(ctrl->handle, &exit_code_dw)){
					exit_code_dw = 0xDEADBEEF;
				}
				*exit_code_ret = (int)exit_code_dw;
			}
			_MCFCRT_CloseThread(ctrl->handle);
			_MCFCRT_free(ctrl);
			break;
		case kStateZombie:
			ctrl->state = kStateJoined;
			joined = true;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_WaitForThreadForever(ctrl->handle);
			if(exit_code_ret){
				unsigned long exit_code_dw;
				if(!GetExitCodeThread(ctrl->handle, &exit_code_dw)){
					exit_code_dw = 0xDEADBEEF;
				}
				*exit_code_ret = (int)exit_code_dw;
			}
			_MCFCRT_CloseThread(ctrl->handle);
			_MCFCRT_free(ctrl);
			break;
		case kStateJoining:
		case kStateJoined:
		case kStateDetached:
			break;
		default:
			_MCFCRT_ASSERT(false);
		}
	}
done:
	_MCFCRT_SignalMutex(&g_ctrlmap_mutex);

	return joined;
}
bool __MCFCRT_C11threadDetach(uintptr_t tid){
	bool detached = false;

	_MCFCRT_WaitForMutexForever(&g_ctrlmap_mutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		ThreadControl *const ctrl = (ThreadControl *)_MCFCRT_AvlFind(&g_ctrlmap, (intptr_t)tid, &ThreadControlComparatorNodeKey);
		if(!ctrl){
			goto done;
		}
		switch(ctrl->state){
		case kStateJoinable:
			ctrl->state = kStateDetached;
			detached = true;
			break;
		case kStateZombie:
			ctrl->state = kStateJoined;
			detached = true;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_CloseThread(ctrl->handle);
			_MCFCRT_free(ctrl);
			break;
		case kStateJoining:
		case kStateJoined:
		case kStateDetached:
			break;
		default:
			_MCFCRT_ASSERT(false);
		}
	}
done:
	_MCFCRT_SignalMutex(&g_ctrlmap_mutex);

	return detached;
}
