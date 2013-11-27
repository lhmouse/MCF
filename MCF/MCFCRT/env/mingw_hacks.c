// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "thread.h"
#include <windows.h>

typedef void (*DTOR)(void *);

typedef struct tagNode {
	DTOR dtor;
	DWORD key;
} NODE;

static void node_dtor(void *mem){
	NODE *const pNode = (NODE *)mem;

	const LPVOID val = TlsGetValue(pNode->key);
	if((GetLastError() == ERROR_SUCCESS) && (val != NULL)){
		(*pNode->dtor)(val);
	}
}

extern int __mingwthr_key_dtor(unsigned long key, DTOR dtor){
	if(dtor == NULL){
		return -1;
	}

	NODE *const pNode = (NODE *)__MCF_CRTRetrieveTls((intptr_t)key, sizeof(NODE), NULL, 0, &node_dtor);
	if(pNode == NULL){
		return -1;
	}

	pNode->dtor	= dtor;
	pNode->key	= key;

	return 0;
}
int __mingwthr_remove_key_dtor(unsigned long key){
	__MCF_CRTDeleteTls((intptr_t)key);
	return 0;
}
