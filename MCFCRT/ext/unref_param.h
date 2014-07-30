// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_UNREF_PARAM_H_
#define MCF_CRT_UNREF_PARAM_H_

#define UNREF_PARAM(param)	((void)(param))

#define UNREF_PARAMS(...)	((void)(__VA_ARGS__))

#endif
