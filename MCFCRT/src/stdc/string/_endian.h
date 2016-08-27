// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_STRING_ENDIAN_H_
#define __MCFCRT_STRING_ENDIAN_H_

#if __UINTPTR_MAX__ == 0xFFFFFFFFFFFFFFFFu
#   define __MCFCRT_BSWAP_UINTPTR              __builtin_bswap64
#elif __UINTPTR_MAX__ == 0xFFFFFFFFu
#   define __MCFCRT_BSWAP_UINTPTR              __builtin_bswap32
#elif __UINTPTR_MAX__ == 0xFFFFu
#   define __MCFCRT_BSWAP_UINTPTR              __builtin_bswap16
#else
#   error Unsupported CPU architecture.
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define __MCFCRT_LOAD_UINTPTR_LE(__src_)             ((__src_))
#define __MCFCRT_SAVE_UINTPTR_LE(__dst_, __src_)     ((__dst_) = (__src_))

#define __MCFCRT_LOAD_UINTPTR_BE(__src_)             (__MCFCRT_BSWAP_UINTPTR(__src_))
#define __MCFCRT_SAVE_UINTPTR_BE(__dst_, __src_)     ((__dst_) = __MCFCRT_BSWAP_UINTPTR(__src_))

#else

#define __MCFCRT_LOAD_UINTPTR_LE(__src_)             (__MCFCRT_BSWAP_UINTPTR(__src_))
#define __MCFCRT_SAVE_UINTPTR_LE(__dst_, __src_)     ((__dst_) = __MCFCRT_BSWAP_UINTPTR(__src_))

#define __MCFCRT_LOAD_UINTPTR_BE(__src_)             ((__src_))
#define __MCFCRT_SAVE_UINTPTR_BE(__dst_, __src_)     ((__dst_) = (__src_))

#endif

#endif
