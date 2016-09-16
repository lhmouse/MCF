// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONFIG_HPP_
#define MCF_CONFIG_HPP_

#ifdef MCF_IMPORT_RTTI
#	define MCF_HAS_EXPORTED_RTTI    __declspec(dllimport)
#else
#	define MCF_HAS_EXPORTED_RTTI    //
#endif

#endif
