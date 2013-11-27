// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_CRTDEF_H__
#define __MCF_CRT_CRTDEF_H__

#ifdef __cplusplus

#	include <cstddef>
#	include <cstdint>

#	define	__MCF_CRT_EXTERN		extern "C" __attribute__((cdecl))
#	define	__MCF_CRT_NOINLINE		extern "C" __attribute__((noinline, cdecl))
#	define	__MCF_EXTERN			extern "C"
#	define	__MCF_STD				::std::

#else

#	include <stddef.h>
#	include <stdint.h>

#	define	__MCF_CRT_EXTERN		extern __attribute__((cdecl))
#	define	__MCF_CRT_NOINLINE		extern __attribute__((noinline, cdecl))
#	define	__MCF_EXTERN			extern
#	define	__MCF_STD

#endif

#endif
