// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MODULE_H_
#define __MCFCRT_ENV_MODULE_H_

#include "_crtdef.h"

#ifndef __MCFCRT_MODULE_INLINE_OR_EXTERN
#	define __MCFCRT_MODULE_INLINE_OR_EXTERN     __attribute__((__gnu_inline__)) extern inline
#endif

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_ModuleInit(void) _MCFCRT_NOEXCEPT;
extern void __MCFCRT_ModuleUninit(void) _MCFCRT_NOEXCEPT;

typedef void (*_MCFCRT_AtModuleExitCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

// ld 自动添加此符号。
extern const char __MCFCRT_ImageBase[]
	__asm__("__image_base__");

__MCFCRT_MODULE_INLINE_OR_EXTERN void *_MCFCRT_GetModuleBase(void) _MCFCRT_NOEXCEPT {
	return (void *)__MCFCRT_ImageBase;
}

typedef struct __MCFCRT_tagModuleSectionInfo {
	char __achName[8];
	_MCFCRT_STD size_t __uRawSize;
	void *__pBase;
	_MCFCRT_STD size_t __uSize;

	struct {
		const void *__pTable;
		_MCFCRT_STD size_t __uCount;
		_MCFCRT_STD size_t __uNext;
		void *__pReserved0;
		void *__pReserved1;
		void *__pReserved2;
	} __vImpl;
} _MCFCRT_ModuleSectionInfo;

extern bool _MCFCRT_EnumerateFirstModuleSection(_MCFCRT_ModuleSectionInfo *__pInfo) _MCFCRT_NOEXCEPT;
extern bool _MCFCRT_EnumerateNextModuleSection(_MCFCRT_ModuleSectionInfo *__pInfo) _MCFCRT_NOEXCEPT;

_MCFCRT_EXTERN_C_END

#endif
