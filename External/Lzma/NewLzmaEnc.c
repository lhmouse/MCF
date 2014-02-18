// Public domain, LH_Mouse. All wrongs reserved.

#include "NewLzmaEnc.h"

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

extern void *__cdecl InitCoroutineProc(void *pStackBottom, void (__cdecl *pfnProc)(void *), void *pParam) __asm__("InitCoroutineProc");
__asm__("InitCoroutineProc: \n"
#ifdef __amd64__
	"lea rax, dword ptr[rcx - 0x110] \n"	// rcx = pStackBottom
	"mov qword ptr[rax + 0xE8], offset x64Wrapper \n"
	"mov qword ptr[rax + 0xF0], rdx \n"		// rdx = pfnProc
	"mov qword ptr[rax + 0xF8], r8 \n"		// r8 = pParam
	"ret \n"

	".align 16 \n"
	"x64Wrapper: \n"
	"	xor rax, rax \n"
	"	push rax \n"
	"	mov rax, qword ptr[rsp + 0x08] \n"
	"	mov rcx, qword ptr[rsp + 0x10] \n"
	"	jmp rax \n"
#else
	"mov ecx, dword ptr[esp + 0x04] \n"		// pStackBottom
	"lea eax, dword ptr[ecx - 0x1C] \n"
	"mov ecx, dword ptr[esp + 0x08] \n"		// pfnProc
	"mov dword ptr[eax + 0x10], ecx \n"
	"xor edx, edx \n"
	"mov dword ptr[eax + 0x14], edx \n"		// Return address of pfnProc. Coroutine procs must not return. Use NULL here.
	"mov ecx, dword ptr[esp + 0x0C] \n"		// pParam
	"mov dword ptr[eax + 0x18], ecx \n"
	"ret \n"
#endif
);

extern void *__fastcall CoroutineSwitchTo(void *sp, void **orig_sp) __asm__("CoroutineSwitchTo");
__asm__("CoroutineSwitchTo: \n"
#ifdef __amd64__
	"sub rsp, 0xA8 \n"
	"push rbx \n"
	"push rbp \n"
	"push rsi \n"
	"push rdi \n"
	"push r12 \n"
	"push r13 \n"
	"push r14 \n"
	"push r15 \n"
	"movdqa xmmword ptr[rsp + 0x40], xmm6  \n"
	"movdqa xmmword ptr[rsp + 0x50], xmm7  \n"
	"movdqa xmmword ptr[rsp + 0x60], xmm8  \n"
	"movdqa xmmword ptr[rsp + 0x70], xmm9  \n"
	"movdqa xmmword ptr[rsp + 0x80], xmm10 \n"
	"movdqa xmmword ptr[rsp + 0x90], xmm11 \n"
	"movdqa xmmword ptr[rsp + 0xA0], xmm12 \n"
	"movdqa xmmword ptr[rsp + 0xB0], xmm13 \n"
	"movdqa xmmword ptr[rsp + 0xC0], xmm14 \n"
	"movdqa xmmword ptr[rsp + 0xD0], xmm15 \n"

	"mov rax, rsp \n"
	"mov rsp, rcx \n"
	"mov qword ptr[rdx], rax \n"

	"movdqa xmm6 , xmmword ptr[rsp + 0x40] \n"
	"movdqa xmm7 , xmmword ptr[rsp + 0x50] \n"
	"movdqa xmm8 , xmmword ptr[rsp + 0x60] \n"
	"movdqa xmm9 , xmmword ptr[rsp + 0x70] \n"
	"movdqa xmm10, xmmword ptr[rsp + 0x80] \n"
	"movdqa xmm11, xmmword ptr[rsp + 0x90] \n"
	"movdqa xmm12, xmmword ptr[rsp + 0xA0] \n"
	"movdqa xmm13, xmmword ptr[rsp + 0xB0] \n"
	"movdqa xmm14, xmmword ptr[rsp + 0xC0] \n"
	"movdqa xmm15, xmmword ptr[rsp + 0xD0] \n"
	"pop r15 \n"
	"pop r14 \n"
	"pop r13 \n"
	"pop r12 \n"
	"pop rdi \n"
	"pop rsi \n"
	"pop rbp \n"
	"pop rbx \n"
	"add rsp, 0xA8 \n"
	"ret \n"
#else
	"push ebx \n"
	"push ebp \n"
	"push esi \n"
	"push edi \n"

	"mov eax, esp \n"
	"mov esp, ecx \n"
	"mov dword ptr[edx], eax \n"

	"pop edi \n"
	"pop esi \n"
	"pop ebp \n"
	"pop ebx \n"
	"ret \n"
#endif
);

typedef struct tagIStream {
	ISeqInStream sis;
	const Byte *src;
	SizeT srcAvail;
} ISTREAM;

typedef struct tagContextHeader {
	CLzmaEncHandle p;
	ICompressProgress *progress;
	ISzAlloc *alloc;
	ISzAlloc *allocBig;

	ISeqOutStream *os;
	ISTREAM is;

	SRes res;

	void *orgSp;
	void *crSp;
} CONTEXT_HEADER;

static SRes ContextRead(void *sis, void *buf, size_t *size){
	ISTREAM *const pIs = (ISTREAM *)sis;

	if(pIs->srcAvail == 0){
		CONTEXT_HEADER *const pHeader = (CONTEXT_HEADER *)((unsigned char *)pIs - offsetof(CONTEXT_HEADER, is));
		pHeader->res = SZ_OK;
		CoroutineSwitchTo(pHeader->orgSp, &pHeader->crSp);
	}
	if(!pIs->src && (pIs->srcAvail != 0)){
		return SZ_ERROR_READ;
	}
	size_t uBytesToRead = pIs->srcAvail;
	if(uBytesToRead > *size){
		uBytesToRead = *size;
	}
	*size = uBytesToRead;

	if(uBytesToRead != 0){
		memcpy(buf, pIs->src, uBytesToRead);
		pIs->src += uBytesToRead;
		pIs->srcAvail -= uBytesToRead;
	}

	return SZ_OK;
}

static __attribute__((noreturn)) void __cdecl CoroutineProc(void *pParam){
	CONTEXT_HEADER *const pHeader = (CONTEXT_HEADER *)pParam;
	for(;;){
		pHeader->res = LzmaEnc_Encode(pHeader->p, pHeader->os, &(pHeader->is.sis), pHeader->progress, pHeader->alloc, pHeader->allocBig);
		CoroutineSwitchTo(pHeader->orgSp, &pHeader->crSp);
	}
}

void *LzmaEnc_NewEncodeCreateContext(CLzmaEncHandle p, ISeqOutStream *outStream, ICompressProgress *progress, ISzAlloc *alloc, ISzAlloc *allocBig){
	SYSTEM_INFO vSystemInfo;
	GetSystemInfo(&vSystemInfo);
	const size_t uPageSize = vSystemInfo.dwPageSize;

	unsigned char *const pContext = (unsigned char *)VirtualAlloc(NULL, 0x10000 + uPageSize * 3, MEM_RESERVE, PAGE_READWRITE);
	if(pContext == NULL){
		return NULL;
	}
	VirtualAlloc(pContext, uPageSize, MEM_COMMIT, PAGE_READWRITE);
	VirtualAlloc(pContext + uPageSize * 2, 0x10000, MEM_COMMIT, PAGE_READWRITE);

	CONTEXT_HEADER *const pHeader = (CONTEXT_HEADER *)pContext;
	pHeader->p = p;
	pHeader->progress = progress;
	pHeader->alloc = alloc;
	pHeader->allocBig = allocBig;
	pHeader->os = outStream;
	pHeader->is.sis.Read = &ContextRead;

	pHeader->crSp = InitCoroutineProc(pContext + uPageSize * 2 + 0x10000, &CoroutineProc, pHeader);

	return pContext;
}
void LzmaEnc_NewEncodeDestroyContext(void *ctx){
	if(ctx != NULL){
		CONTEXT_HEADER *const pHeader = (CONTEXT_HEADER *)ctx;
		pHeader->is.src = NULL;
		pHeader->is.srcAvail = 1;
		CoroutineSwitchTo(pHeader->crSp, &pHeader->orgSp);

		VirtualFree(ctx, 0, MEM_RELEASE);
	}
}

SRes LzmaEnc_NewEncode(void *ctx, const Byte *src, SizeT srcLen){
	CONTEXT_HEADER *const pHeader = (CONTEXT_HEADER *)ctx;
	pHeader->is.src = src;
	pHeader->is.srcAvail = srcLen;
	CoroutineSwitchTo(pHeader->crSp, &pHeader->orgSp);

	return pHeader->res;
}
