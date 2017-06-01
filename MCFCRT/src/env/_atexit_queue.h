// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_ATEXIT_QUEUE_H_
#define __MCFCRT_ENV_ATEXIT_QUEUE_H_

#include "_crtdef.h"
#include "mutex.h"
#include "heap.h"

_MCFCRT_EXTERN_C_BEGIN

typedef struct __MCFCRT_tagAtExitElement {
	void (*__proc)(_MCFCRT_STD intptr_t);
	_MCFCRT_STD intptr_t __context;
} __MCFCRT_AtExitElement;

typedef struct __MCFCRT_tagAtExitQueueNode {
	struct __MCFCRT_tagAtExitQueueNode *__prev;
	_MCFCRT_STD size_t __size;
	__MCFCRT_AtExitElement __data[62];
} __MCFCRT_AtExitQueueNode;

typedef struct __MCFCRT_tagAtExitQueue {
	_MCFCRT_Mutex __mutex;
	__MCFCRT_AtExitQueueNode *__last;
	__MCFCRT_AtExitQueueNode __spare;
} __MCFCRT_AtExitQueue;

static_assert(sizeof(__MCFCRT_AtExitQueue) % 256 == 0, "??");

static inline bool __MCFCRT_AtExitQueuePush(__MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __queue, const __MCFCRT_AtExitElement *_MCFCRT_RESTRICT __elem) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__queue->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__node, *__next;
		__node = __queue->__last;
		if(!__node){
			__node = &(__queue->__spare);
		}
		if(__node->__size >= sizeof(__node->__data) / sizeof(__node->__data[0])){
			__next = _MCFCRT_malloc(sizeof(*__next));
			if(!__next){
				_MCFCRT_SignalMutex(&(__queue->__mutex));
				return false;
			}
			__next->__prev = __node;
			__next->__size = 0;
			__node = __next;
		}
		__node->__data[(__node->__size)++] = *__elem;
		__queue->__last = __node;
	}
	_MCFCRT_SignalMutex(&(__queue->__mutex));
	return true;
}
static inline bool __MCFCRT_AtExitQueuePop(__MCFCRT_AtExitElement *_MCFCRT_RESTRICT __elem, __MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __queue) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__queue->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__node, *__prev;
		__node = __queue->__last;
		if(!__node){
			__node = &(__queue->__spare);
		}
		if(__node->__size == 0){
			_MCFCRT_SignalMutex(&(__queue->__mutex));
			return false;
		}
		*__elem = __node->__data[--(__node->__size)];
		if((__node->__size == 0) && (__node != &(__queue->__spare))){
			__prev = __node->__prev;
			_MCFCRT_free(__node);
			__node = __prev;
		}
		__queue->__last = __node;
	}
	_MCFCRT_SignalMutex(&(__queue->__mutex));
	return true;
}
static inline void __MCFCRT_AtExitQueueClear(__MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __queue) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__queue->__mutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__node, *__prev;
		__node = __queue->__last;
		if(!__node){
			__node = &(__queue->__spare);
		}
		while(__node != &(__queue->__spare)){
			__prev = __node->__prev;
			_MCFCRT_free(__node);
			__node = __prev;
		}
		__queue->__spare.__size = 0;
		__queue->__last = __node;
	}
	_MCFCRT_SignalMutex(&(__queue->__mutex));
}

_MCFCRT_EXTERN_C_END

#endif
