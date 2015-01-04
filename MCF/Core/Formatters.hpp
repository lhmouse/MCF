// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FORMATTERS_HPP_
#define MCF_CORE_FORMATTERS_HPP_

namespace MCF {

namespace Formatters {
}

template<typename ...FormattersT>
struct Printer {
};

template<typename ...FormattersT>
struct Scanner {
};

}

#define PRINTER(...)	\
	[&]{ using namespace ::MCF::Formatters; return ::MCF::Printer<__VA_ARGS__>(); }()

#define SCANNER(...)	\
	[&]{ using namespace ::MCF::Formatters; return ::MCF::Scanner<__VA_ARGS__>(); }()

#endif
