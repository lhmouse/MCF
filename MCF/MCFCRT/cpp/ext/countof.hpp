// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRTCOUNTOF_HPP__
#define __MCF_CRTCOUNTOF_HPP__

#include <cstddef>

namespace MCF {
	namespace __MCF {
		template<typename T, std::size_t N>
		char (*CountOfHelper(T (&)[N]))[N];

		template<typename T, std::size_t N>
		char (*CountOfHelper(T (&&)[N]))[N];
	}
}

#define COUNTOF(ar)	(sizeof(*::MCF::__MCF::CountOfHelper((ar))))

#endif
