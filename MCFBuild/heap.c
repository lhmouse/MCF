// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN 1
#include "heap.h"
#include <windows.h>

void *MCFBUILD_HeapAlloc(size_t uSize){
	return HeapAlloc(GetProcessHeap(), 0, uSize);
}
void *MCFBUILD_HeapRealloc(void *pBlock, size_t uSize){
	if(!pBlock){
		return HeapAlloc(GetProcessHeap(), 0, uSize);
	}
	return HeapReAlloc(GetProcessHeap(), 0, pBlock, uSize);
}
void MCFBUILD_HeapFree(void *pBlock){
	if(!pBlock){
		return;
	}
	HeapFree(GetProcessHeap(), 0, pBlock);
}
