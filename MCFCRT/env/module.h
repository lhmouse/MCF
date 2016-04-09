// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_MODULE_H_
#define __MCFCRT_ENV_MODULE_H_

#include "_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

// __MCFCRT_BeginModule() 在所有 CRT 功能都初始化成功后返回 true。
extern bool __MCFCRT_BeginModule(void);
extern void __MCFCRT_EndModule(void);

typedef void (*_MCFCRT_AtEndModuleCallback)(_MCFCRT_STD intptr_t __nContext);

extern bool _MCFCRT_AtEndModule(_MCFCRT_AtEndModuleCallback __pfnProc, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

extern void *_MCFCRT_GetModuleBase(void) _MCFCRT_NOEXCEPT;

// __pchName 指向 8 个字节固定长度的名称。
// 返回 true 则继续遍历，否则终止遍历。
typedef bool (*_MCFCRT_TraverseModuleSectionsCallback)(_MCFCRT_STD intptr_t __nContext, const char *__pchName, _MCFCRT_STD size_t __uRawSize, void *__pBase, _MCFCRT_STD size_t __uSize);

// 只要完成了至少一次遍历就会返回 true。
extern bool _MCFCRT_TraverseModuleSections(_MCFCRT_TraverseModuleSectionsCallback __pfnCallback, _MCFCRT_STD intptr_t __nContext) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#endif
