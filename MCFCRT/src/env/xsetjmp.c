// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "xsetjmp.h"

__attribute__((__noreturn__))
void __MCFCRT_longjmp_wrapper(void **env){
	__builtin_longjmp(env, 1);
}
