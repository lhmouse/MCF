// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_PP_H_
#define __MCFCRT_EXT_PP_H_

#define __MCFCRT_PP_STRINGIFY_(...)             # __VA_ARGS__
#define _MCFCRT_PP_STRINGIFY(...)               __MCFCRT_PP_STRINGIFY_(__VA_ARGS__)

#define __MCFCRT_PP_FIRST_(__f_, ...)           __f_
#define _MCFCRT_PP_FIRST(...)                   __MCFCRT_PP_FIRST_(__VA_ARGS__)

#define __MCFCRT_PP_REST_(__f_, ...)            __VA_ARGS__
#define _MCFCRT_PP_REST(...)                    __MCFCRT_PP_REST_(__VA_ARGS__)

#define __MCFCRT_PP_CAT2_(__x_, __y_)           __x_ ## __y_
#define _MCFCRT_PP_CAT2(__x_, __y_)             __MCFCRT_PP_CAT2_(__x_, __y_)

#define __MCFCRT_PP_CAT3_(__x_, __y_, __z_)     __x_ ## __y_ ## __z_
#define _MCFCRT_PP_CAT3(__x_, __y_, __z_)       __MCFCRT_PP_CAT3_(__x_, __y_, __z_)

#endif
