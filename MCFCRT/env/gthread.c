// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "gthread.h"
#include "avl_tree.h"
#include "heap.h"
#include "mutex.h"
#include "condition_variable.h"
#include "eh_top.h"

void __MCFCRT_GthreadTlsDestructor(intptr_t context, void *storage){
	void (*const destructor)(void *) = (void (*)(void *))context;

	void *const value = *(void **)storage;
	if(!value){
		return;
	}
	(*destructor)(value);
}

intptr_t __MCFCRT_GthreadUnlockCallbackMutex(intptr_t context){
	__gthread_mutex_t *const mutex = (__gthread_mutex_t *)context;

	__gthread_mutex_unlock(mutex);
	return 1;
}
void __MCFCRT_GthreadRelockCallbackMutex(intptr_t context, intptr_t unlocked){
	__gthread_mutex_t *const mutex = (__gthread_mutex_t *)context;

	_MCFCRT_ASSERT((size_t)unlocked == 1);
	__gthread_mutex_lock(mutex);
}

intptr_t __MCFCRT_GthreadUnlockCallbackRecursiveMutex(intptr_t context){
	__gthread_recursive_mutex_t *const recur_mutex = (__gthread_recursive_mutex_t *)context;
	_MCFCRT_ASSERT(_MCFCRT_GetCurrentThreadId() == __atomic_load_n(&(recur_mutex->__owner), __ATOMIC_RELAXED));

	const size_t old_count = recur_mutex->__count;
	recur_mutex->__count = 0;
	__atomic_store_n(&(recur_mutex->__owner), 0, __ATOMIC_RELAXED);

	__gthread_mutex_unlock(&(recur_mutex->__mutex));
	return (intptr_t)old_count;
}
void __MCFCRT_GthreadRelockCallbackRecursiveMutex(intptr_t context, intptr_t unlocked){
	__gthread_recursive_mutex_t *const recur_mutex = (__gthread_recursive_mutex_t *)context;

	_MCFCRT_ASSERT((size_t)unlocked >= 1);
	__gthread_mutex_lock(&(recur_mutex->__mutex));

	const uintptr_t self = _MCFCRT_GetCurrentThreadId();
	__atomic_store_n(&(recur_mutex->__owner), self, __ATOMIC_RELAXED);
	recur_mutex->__count = (size_t)unlocked;
}

static __gthread_mutex_t g_ctrlmap_mutex = { 0 };
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

	void *(*proc)(void *);
	void *param;

	ThreadState state;
	void *exit_code;
	__gthread_cond_t termination;

	uintptr_t tid;
	_MCFCRT_ThreadHandle handle;
} ThreadControl;

static inline int ThreadControlComparatorNodeKey(const _MCFCRT_AvlNodeHeader *lhs, intptr_t rhs){
	const uintptr_t u1 = (uintptr_t)(((const ThreadControl *)lhs)->tid);
	const uintptr_t u2 = (uintptr_t)(void *)rhs;
	return (u1 < u2) ? -1 : ((u1 > u2) ? 1 : 0);
}
static inline int ThreadControlComparatorNodes(const _MCFCRT_AvlNodeHeader *lhs, const _MCFCRT_AvlNodeHeader *pObj2){
	return ThreadControlComparatorNodeKey(lhs, (intptr_t)(((const ThreadControl *)pObj2)->tid));
}

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
unsigned long GthreadProc(void *ctrl_ptr){
	ThreadControl *const ctrl = ctrl_ptr;
	_MCFCRT_ASSERT(ctrl);

	void *(*const proc)(void *) = ctrl->proc;
	void *const param = ctrl->param;

	void *exit_code;

	__MCFCRT_EH_TOP_BEGIN
	{
		exit_code = (*proc)(param);
	}
	__MCFCRT_EH_TOP_END

	__gthread_mutex_lock(&g_ctrlmap_mutex);
	{
		switch(ctrl->state){
		case kStateJoinable:
			ctrl->state = kStateZombie;
			ctrl->exit_code = exit_code;
			break;
		case kStateZombie:
			_MCFCRT_ASSERT(false);
		case kStateJoining:
			ctrl->state = kStateJoined;
			ctrl->exit_code = exit_code;
			__gthread_cond_broadcast(&(ctrl->termination));
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
	__gthread_mutex_unlock(&g_ctrlmap_mutex);

	return 0;
}

uintptr_t __MCFCRT_GthreadCreateJoinable(void *(*proc)(void *), void *param){
	ThreadControl *const ctrl = _MCFCRT_malloc(sizeof(ThreadControl));
	if(!ctrl){
		return 0;
	}
	ctrl->proc      = proc;
	ctrl->param     = param;
	ctrl->state     = kStateJoinable;
	ctrl->exit_code = (void *)0xDEADBEEF;
	__gthread_cond_init_function(&(ctrl->termination));

	uintptr_t tid;
	const _MCFCRT_ThreadHandle handle = _MCFCRT_CreateNativeThread(&GthreadProc, ctrl, true, &tid);
	if(!handle){
		 _MCFCRT_free(ctrl);
		return 0;
	}
	ctrl->tid    = tid;
	ctrl->handle = handle;

	__gthread_mutex_lock(&g_ctrlmap_mutex);
	{
		_MCFCRT_AvlAttach(&g_ctrlmap, (_MCFCRT_AvlNodeHeader *)ctrl, &ThreadControlComparatorNodes);
	}
	__gthread_mutex_unlock(&g_ctrlmap_mutex);

	_MCFCRT_ResumeThread(handle);
	return tid;
}
bool __MCFCRT_GthreadJoin(uintptr_t tid, void **restrict exit_code_ret){
	bool joined = false;

	__gthread_mutex_lock(&g_ctrlmap_mutex);
	{
		ThreadControl *const ctrl = (ThreadControl *)_MCFCRT_AvlFind(&g_ctrlmap, (intptr_t)tid, &ThreadControlComparatorNodeKey);
		if(!ctrl){
			goto done;
		}
		switch(ctrl->state){
		case kStateJoinable:
			ctrl->state = kStateJoining;
			do {
				__gthread_cond_wait(&(ctrl->termination), &g_ctrlmap_mutex);
			} while(ctrl->state != kStateJoined);
			if(exit_code_ret){
				*exit_code_ret = ctrl->exit_code;
			}
			joined = true;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_WaitForThreadForever(ctrl->handle);
			_MCFCRT_CloseThread(ctrl->handle);
			_MCFCRT_free(ctrl);
			break;
		case kStateZombie:
			ctrl->state = kStateJoined;
			if(exit_code_ret){
				*exit_code_ret = ctrl->exit_code;
			}
			joined = true;
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)ctrl);
			_MCFCRT_WaitForThreadForever(ctrl->handle);
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
	__gthread_mutex_unlock(&g_ctrlmap_mutex);

	return joined;
}
bool __MCFCRT_GthreadDetach(uintptr_t tid){
	bool detached = false;

	__gthread_mutex_lock(&g_ctrlmap_mutex);
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
	__gthread_mutex_unlock(&g_ctrlmap_mutex);

	return detached;
}
