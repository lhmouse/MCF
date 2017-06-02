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
	void (*__pfnProc)(_MCFCRT_STD intptr_t);
	_MCFCRT_STD intptr_t __nContext;
} __MCFCRT_AtExitElement;

typedef struct __MCFCRT_tagAtExitQueueNode {
	struct __MCFCRT_tagAtExitQueueNode *__pPrev;
	_MCFCRT_STD size_t __uSize;
	__MCFCRT_AtExitElement __aData[62];
} __MCFCRT_AtExitQueueNode;

typedef struct __MCFCRT_tagAtExitQueue {
	_MCFCRT_Mutex __vMutex;
	__MCFCRT_AtExitQueueNode *__pLast;
	__MCFCRT_AtExitQueueNode __vSpare;
} __MCFCRT_AtExitQueue;

#if defined(_MCFCRT_C11) || defined(_MCFCRT_CXX11)
static_assert(sizeof(__MCFCRT_AtExitQueue) % 256 == 0, "??");
#endif

static inline bool __MCFCRT_AtExitQueuePush(__MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __pQueue, const __MCFCRT_AtExitElement *_MCFCRT_RESTRICT __pElement) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__pQueue->__vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__pNode, *__pNext;
		__pNode = __pQueue->__pLast;
		if(!__pNode){
			__pNode = &(__pQueue->__vSpare);
		}
		if(__pNode->__uSize >= sizeof(__pNode->__aData) / sizeof(__pNode->__aData[0])){
			__pNext = (__MCFCRT_AtExitQueueNode *)_MCFCRT_malloc(sizeof(*__pNext));
			if(!__pNext){
				_MCFCRT_SignalMutex(&(__pQueue->__vMutex));
				return false;
			}
			__pNext->__pPrev = __pNode;
			__pNext->__uSize = 0;
			__pNode = __pNext;
		}
		__pNode->__aData[(__pNode->__uSize)++] = *__pElement;
		__pQueue->__pLast = __pNode;
	}
	_MCFCRT_SignalMutex(&(__pQueue->__vMutex));
	return true;
}
static inline bool __MCFCRT_AtExitQueuePop(__MCFCRT_AtExitElement *_MCFCRT_RESTRICT __pElement, __MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __pQueue) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__pQueue->__vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__pNode, *__pPrev;
		__pNode = __pQueue->__pLast;
		if(!__pNode){
			__pNode = &(__pQueue->__vSpare);
		}
		if(__pNode->__uSize == 0){
			_MCFCRT_SignalMutex(&(__pQueue->__vMutex));
			return false;
		}
		*__pElement = __pNode->__aData[--(__pNode->__uSize)];
		if((__pNode->__uSize == 0) && (__pNode != &(__pQueue->__vSpare))){
			__pPrev = __pNode->__pPrev;
			_MCFCRT_free(__pNode);
			__pNode = __pPrev;
		}
		__pQueue->__pLast = __pNode;
	}
	_MCFCRT_SignalMutex(&(__pQueue->__vMutex));
	return true;
}
static inline void __MCFCRT_AtExitQueueClear(__MCFCRT_AtExitQueue *_MCFCRT_RESTRICT __pQueue) _MCFCRT_NOEXCEPT {
	_MCFCRT_WaitForMutexForever(&(__pQueue->__vMutex), _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		__MCFCRT_AtExitQueueNode *__pNode, *__pPrev;
		__pNode = __pQueue->__pLast;
		if(!__pNode){
			__pNode = &(__pQueue->__vSpare);
		}
		while(__pNode != &(__pQueue->__vSpare)){
			__pPrev = __pNode->__pPrev;
			_MCFCRT_free(__pNode);
			__pNode = __pPrev;
		}
		__pQueue->__vSpare.__uSize = 0;
		__pQueue->__pLast = __pNode;
	}
	_MCFCRT_SignalMutex(&(__pQueue->__vMutex));
}

static inline void __MCFCRT_AtExitQueueInvoke(const __MCFCRT_AtExitElement *_MCFCRT_RESTRICT __pElement) _MCFCRT_NOEXCEPT {
	(*(__pElement->__pfnProc))(__pElement->__nContext);
}

_MCFCRT_EXTERN_C_END

#endif
