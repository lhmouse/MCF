// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_COUNT_OF_HPP_
#define MCF_CRT_COUNT_OF_HPP_

#include <cstddef>

namespace MCF {
	template<typename T, std::size_t N>
	char (*CountOfHelper_(T (&)[N]))[N];

	template<typename T, std::size_t N>
	char (*CountOfHelper_(T (&&)[N]))[N];
}

#define COUNT_OF(ar)	(sizeof(*::MCF::CountOfHelper_((ar))))

#endif
