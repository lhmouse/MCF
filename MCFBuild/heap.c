// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN 1
#include "heap.h"
#include "last_error.h"
#include <windows.h>

void *MCFBUILD_HeapAlloc(size_t uSize){
	void *pBlock = HeapAlloc(GetProcessHeap(), 0, uSize);
	if(!pBlock){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}
	return pBlock;
}
void *MCFBUILD_HeapRealloc(void *pBlockOld, size_t uSize){
	if(!pBlockOld){
		// Do not pass null pointers to `HeapReAlloc()`.
		return MCFBUILD_HeapAlloc(uSize);
	}
	void *pBlock = HeapReAlloc(GetProcessHeap(), 0, pBlockOld, uSize);
	if(!pBlock){
		MCFBUILD_SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return 0;
	}
	return pBlock;
}
void MCFBUILD_HeapFree(void *pBlock){
	if(!pBlock){
		// Do not pass null pointers to `HeapFree()`.
		return;
	}
	HeapFree(GetProcessHeap(), 0, pBlock);
}
