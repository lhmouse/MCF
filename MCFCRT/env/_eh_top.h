// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_EH_TOP_H_
#define MCF_CRT_EH_TOP_H_

#include "_crtdef.h"

#ifdef __SEH__

#	define __MCF_EH_TOP_BEGIN	\
		__asm__ __volatile__("seh_try: \n");

#	define __MCF_EH_TOP_END	\
		__asm__ __volatile__(	\
			"seh_except: \n"	\
			"	.seh_handler __C_specific_handler, @except \n"	\
			"	.seh_handlerdata \n"	\
			"	.long 1 \n"	\
			"	.rva seh_try, seh_except, _gnu_exception_handler, seh_except \n"	\
			"	.text \n"	\
		);

#else

#	define __MCF_EH_TOP_BEGIN
#	define __MCF_EH_TOP_END

#endif

#endif
