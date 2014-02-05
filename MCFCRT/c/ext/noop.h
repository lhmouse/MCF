// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_NOOP_H__
#define __MCF_CRT_NOOP_H__

static inline void __MCF_CRT_Noop(int unused __attribute__((unused)), ...){
}

#define NOOP(...)	(__MCF_CRT_Noop(0, __VA_ARGS__))

#endif
