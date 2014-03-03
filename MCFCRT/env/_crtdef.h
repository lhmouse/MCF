// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_CRTDEF_H__
#define __MCF_CRT_CRTDEF_H__

#ifdef __cplusplus
#	include <cstddef>
#	include <cstdint>
#	include <climits>
#	include <cstdbool>
#else
#	include <stddef.h>
#	include <stdint.h>
#	include <limits.h>
#	include <stdbool.h>
#endif

#ifdef __cplusplus
#	define	__MCF_EXTERN_C_BEGIN	extern "C" {
#	define	__MCF_EXTERN_C_END		}
#	define	__MCF_STD				::std::
#	define	__MCF_NOEXCEPT			noexcept
#else
#	define	__MCF_EXTERN_C_BEGIN
#	define	__MCF_EXTERN_C_END
#	define	__MCF_STD
#	define	__MCF_NOEXCEPT
#endif

// C++11 目前还不支持 C99 的 restrict 限定符。
#ifdef __cplusplus
#	define	restrict				__restrict__
#endif

#endif
