// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MODULE_H_
#define __MCFCRT_ENV_MODULE_H_

#include "_crtdef.h"

_MCFCRT_EXTERN_C_BEGIN

extern bool __MCFCRT_BeginModule(void);
extern void __MCFCRT_EndModule(void);

typedef void (*_MCFCRT_AtEndModuleCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtEndModule(_MCFCRT_AtEndModuleCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern void *_MCFCRT_GetModuleBase(void) _MCFCRT_NOEXCEPT;

typedef struct _MCFCRT_tagModuleSectionInfo {
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
