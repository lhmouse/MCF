// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_COMMON_H_
#define MCFBUILD_COMMON_H_

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdbool.h>
#include <stdalign.h>
#include <uchar.h>
#include <assert.h>

#ifdef __cplusplus
#  define MCFBUILD_EXTERN_C_BEGIN       extern "C" {
#  define MCFBUILD_EXTERN_C_END         }
#  define MCFBUILD_STD                  ::
#  define MCFBUILD_NOEXCEPT             throw()
#  define MCFBUILD_RESTRICT             __restrict__
#else
#  define MCFBUILD_EXTERN_C_BEGIN       //
#  define MCFBUILD_EXTERN_C_END         //
#  define MCFBUILD_STD                  //
#  define MCFBUILD_NOEXCEPT             //
#  define MCFBUILD_RESTRICT             restrict
#endif

#endif
