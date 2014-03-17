// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_COUNTOF_HPP__
#define __MCF_CRT_COUNTOF_HPP__

#include <cstddef>

namespace MCF {

namespace __MCF {
	template<typename T, std::size_t N>
	auto CountOfHelper(T (&)[N]) -> char (&)[N];

	template<typename T, std::size_t N>
	auto CountOfHelper(T (&&)[N]) -> char (&)[N];
}

}

#define COUNT_OF(ar)	(sizeof(::MCF::__MCF::CountOfHelper(ar)))

#endif
