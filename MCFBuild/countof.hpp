// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_COUNTOF_HPP__
#define __MCF_CRT_COUNTOF_HPP__

#include <cstddef>

namespace __MCF {
	template<typename T, std::size_t N>
	char (*countof_helper(T (&)[N]))[N];

	template<typename T, std::size_t N>
	char (*countof_helper(T (&&)[N]))[N];
}

#define COUNTOF(ar)	(sizeof(*::__MCF::countof_helper((ar))))

#endif
